"""
In-memory appointment DB + resolution strategies
==================================================

When the visitor does not know something (forgot host, fuzzy date…), the
**application** must look it up — not the LLM, and especially not the visitor.

Business functions return a **BusinessOutcome** — visitor message plus optional staff pager.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from datetime import date, datetime, timedelta
from typing import Optional

from guichet_router import FrontDeskDecision


# -----------------------------------------------------------------------------
# Data model
# -----------------------------------------------------------------------------


@dataclass(frozen=True)
class Appointment:
    visitor: str
    host: str
    starts_at: datetime
    subject: str = ""


@dataclass
class BusinessOutcome:
    """Desk reply and whether reception should be paged (for staff simulation)."""

    visitor_message: str
    staff_requested: bool = False
    staff_brief: str = ""
    notify_host: bool = False
    host_name: str = ""
    visitor_name: str = ""
    appointment_at: Optional[datetime] = None
    appointment_subject: str = ""


def _today_at(
    hour: int,
    minute: int = 0,
    *,
    days: int = 0,
    ref: Optional[datetime] = None,
) -> datetime:
    base = (ref or datetime.now().astimezone()).replace(
        hour=hour, minute=minute, second=0, microsecond=0
    )
    return base + timedelta(days=days)


def _seed_appointments(*, ref: Optional[datetime] = None) -> list[Appointment]:
    return [
        Appointment("Thomas Martin", "Julie", _today_at(20, 0, ref=ref), "Project meeting"),
        Appointment("Sarah Bernard", "Mr Martin", _today_at(15, 30, ref=ref), "HR interview"),
        Appointment("Marc Durant", "Dr Bernard", _today_at(11, 0, days=1, ref=ref), "Medical follow-up"),
        Appointment("Marie Dubois", "Dr Lemoine", _today_at(14, 30, days=1, ref=ref), "Consultation"),
        Appointment("Léa Petit", "Mr Martin", _today_at(9, 0, days=2, ref=ref), "Onboarding"),
    ]


APPOINTMENTS: list[Appointment] = _seed_appointments()


def reset_calendar(*, now: Optional[datetime] = None) -> None:
    """Restore seed data — call at demo start for reproducible runs."""
    ref = now or datetime.now().astimezone()
    APPOINTMENTS.clear()
    APPOINTMENTS.extend(_seed_appointments(ref=ref))


# -----------------------------------------------------------------------------
# DB queries
# -----------------------------------------------------------------------------


def _same_name(a: str, b: str) -> bool:
    return a.strip().casefold() == b.strip().casefold()


def _is_blank(s: Optional[str]) -> bool:
    return s is None or not s.strip()


def _fmt_time(dt: datetime) -> str:
    return dt.strftime("%A at %H:%M")


def _fmt_date(dt: datetime) -> str:
    return dt.strftime("%A %d %B")


# Check-in "take a seat" only when the slot is imminent (±20 min).
_ARRIVAL_WINDOW = timedelta(minutes=20)


def _arrival_kind(appointment_at: datetime, now: datetime) -> str:
    """How the appointment time relates to the desk clock right now."""
    delta = appointment_at - now
    if delta < -_ARRIVAL_WINDOW:
        return "past"
    if delta <= _ARRIVAL_WINDOW:
        return "imminent"
    if appointment_at.date() == now.date():
        return "later_today"
    return "future"


def find_by_visitor(
    name: str,
    *,
    around: Optional[datetime] = None,
    tolerance_h: float = 36.0,
) -> list[Appointment]:
    hits = [a for a in APPOINTMENTS if _same_name(a.visitor, name)]
    if around is None:
        return hits
    delta = timedelta(hours=tolerance_h)
    return [a for a in hits if abs(a.starts_at - around) <= delta]


def _find_exact(
    visitor_name: str, host_name: str, appointment_at: datetime
) -> Optional[Appointment]:
    for a in APPOINTMENTS:
        if (
            _same_name(a.visitor, visitor_name)
            and _same_name(a.host, host_name)
            and a.starts_at == appointment_at
        ):
            return a
    return None


def _resolve_check_in(
    visitor_name: str,
    host_name: Optional[str],
    hinted_at: Optional[datetime],
    now: datetime,
) -> Optional[Appointment]:
    """Find a calendar row for check-in even when the LLM time is wrong (e.g. now_iso).

    Typical case: visitor says « I'm here now » at 21:30 but the booking was at 20:00.
    We match on visitor (+ host if known) and the relevant calendar day — not exact time.
    """
    candidates = find_by_visitor(visitor_name)
    if not candidates:
        return None

    if not _is_blank(host_name):
        by_host = [a for a in candidates if _same_name(a.host, host_name)]
        if by_host:
            candidates = by_host

    ref_day = (hinted_at or now).date()
    same_day = [a for a in candidates if a.starts_at.date() == ref_day]
    if not same_day:
        same_day = [a for a in candidates if a.starts_at.date() == now.date()]

    if len(same_day) == 1:
        return same_day[0]
    if len(candidates) == 1:
        return candidates[0]
    if same_day:
        # Closest slot to desk clock (e.g. today's passed appointment vs another).
        return min(same_day, key=lambda a: abs((a.starts_at - now).total_seconds()))
    return None


# -----------------------------------------------------------------------------
# Strategy result
# -----------------------------------------------------------------------------


@dataclass
class StrategyResult:
    host_name: Optional[str] = None
    appointment_at: Optional[datetime] = None
    appointment_found: Optional[Appointment] = None
    log: list[str] = field(default_factory=list)


_TOMORROW_WORDS = ("tomorrow", "demain", "morgen", "mañana")
_TODAY_WORDS = ("today", "aujourd'hui", "heute", "hoy")
_LOOKUP_INTENTS = ("check_in", "book_appointment")


def _strategy_calendar_lookup(
    decision: FrontDeskDecision, now: datetime, result: StrategyResult
) -> None:
    """Fill missing fields from calendar, or reconcile check-in with the real row."""
    if decision.intent not in _LOOKUP_INTENTS:
        return
    if _is_blank(decision.visitor_name):
        return

    # check_in: always reconcile — LLM often sets appointment_at to now_iso.
    if decision.intent == "check_in":
        appt = _resolve_check_in(
            decision.visitor_name,
            decision.host_name,
            decision.appointment_at,
            now,
        )
        if appt:
            result.appointment_found = appt
            result.host_name = appt.host
            result.appointment_at = appt.starts_at
            result.log.append(
                f"calendar: {appt.host} at {appt.starts_at.strftime('%d/%m %H:%M')} "
                f"({appt.subject})"
            )
        else:
            result.log.append(
                f"no appointment in calendar for {decision.visitor_name}."
            )
        return

    needs_host = _is_blank(decision.host_name)
    needs_time = decision.appointment_at is None
    if not needs_host and not needs_time:
        return

    around = decision.appointment_at or now
    matches = find_by_visitor(decision.visitor_name, around=around)

    if not needs_host and needs_time:
        matches = [
            a for a in matches if _same_name(a.host, decision.host_name)
        ]

    if len(matches) == 1:
        appt = matches[0]
        if needs_host:
            result.host_name = appt.host
        # For check_in, always take calendar time (LLM may have filled now_iso).
        if needs_time or decision.intent == "check_in":
            result.appointment_at = appt.starts_at
        result.appointment_found = appt
        parts = []
        if needs_host:
            parts.append(f"host={appt.host}")
        if needs_time:
            parts.append(f"time={appt.starts_at.strftime('%d/%m %H:%M')}")
        result.log.append(
            f"calendar lookup: {', '.join(parts)} "
            f"(subject: {appt.subject})"
        )
    elif len(matches) > 1:
        result.log.append(
            f"{len(matches)} appointments for {decision.visitor_name}: "
            "staff must disambiguate."
        )
    else:
        result.log.append(
            f"no appointment in calendar for {decision.visitor_name}."
        )


def _strategy_cancellation(
    decision: FrontDeskDecision, now: datetime, result: StrategyResult
) -> None:
    if decision.intent != "cancel_appointment" or _is_blank(decision.visitor_name):
        return

    candidates = find_by_visitor(decision.visitor_name)
    if not candidates:
        result.log.append(
            f"no appointment to cancel for {decision.visitor_name}."
        )
        return

    # Single row → use it (no day filter needed).
    if len(candidates) == 1:
        appt = candidates[0]
        result.appointment_found = appt
        result.log.append(
            f"found: {appt.visitor} with {appt.host} "
            f"on {_fmt_time(appt.starts_at)}."
        )
        return

    # Multiple rows → narrow by day hint if provided.
    hint = (decision.cancel_date_text or "").casefold()
    filtered = candidates
    if any(w in hint for w in _TOMORROW_WORDS):
        day = (now + timedelta(days=1)).date()
        filtered = [c for c in candidates if c.starts_at.date() == day]
    elif any(w in hint for w in _TODAY_WORDS):
        day = now.date()
        filtered = [c for c in candidates if c.starts_at.date() == day]

    if len(filtered) == 1:
        appt = filtered[0]
        result.appointment_found = appt
        result.log.append(
            f"found (day filter): {appt.visitor} with {appt.host} "
            f"on {_fmt_time(appt.starts_at)}."
        )
    elif len(filtered) == 0:
        result.log.append(
            f"no appointment matching {decision.visitor_name!r} "
            f"for {decision.cancel_date_text!r}."
        )
    else:
        result.log.append(
            f"{len(filtered)} matches — staff must pick one."
        )


STRATEGIES = (_strategy_calendar_lookup, _strategy_cancellation)


def needs_strategy_resolution(decision: FrontDeskDecision) -> bool:
    if decision.intent == "check_in" and not _is_blank(decision.visitor_name):
        return True  # always reconcile with calendar (LLM time is often wrong)
    if decision.intent == "book_appointment" and not _is_blank(decision.visitor_name):
        return decision.needs_app_assistance
    if decision.intent == "cancel_appointment" and not _is_blank(decision.visitor_name):
        return True
    return decision.needs_app_assistance


def apply_strategies(
    decision: FrontDeskDecision, *, now: Optional[datetime] = None
) -> StrategyResult:
    moment = now or datetime.now().astimezone()
    result = StrategyResult()
    for strategy in STRATEGIES:
        strategy(decision, moment, result)
    return result


# -----------------------------------------------------------------------------
# Business functions
# -----------------------------------------------------------------------------


def _merged_fields(
    decision: FrontDeskDecision, result: Optional[StrategyResult]
) -> tuple[Optional[str], Optional[str], Optional[datetime], Optional[Appointment]]:
    """Calendar strategy wins over LLM guesses (especially for check_in)."""
    appt = result.appointment_found if result else None
    if appt and decision.intent == "check_in":
        return decision.visitor_name, appt.host, appt.starts_at, appt
    host = (result.host_name if result else None) or decision.host_name
    when = (result.appointment_at if result else None) or decision.appointment_at
    return decision.visitor_name, host, when, appt


def check_in_visitor(
    visitor_name: str,
    host_name: str,
    appointment_at: datetime,
    *,
    now: Optional[datetime] = None,
    subject: str = "",
) -> str:
    """Verify an existing appointment and welcome the visitor (no DB insert)."""
    print("[Business] check_in_visitor(")
    print(f"    visitor_name={visitor_name!r},")
    print(f"    host_name={host_name!r},")
    print(f"    appointment_at={appointment_at!r},")
    print(")")

    clock = now or datetime.now().astimezone()
    appt = _find_exact(visitor_name, host_name, appointment_at)
    if appt is None:
        appt = _resolve_check_in(visitor_name, host_name, appointment_at, clock)
    if appt is None:
        return (
            f"I'm sorry {visitor_name}, I cannot find an appointment under that name. "
            f"A staff member will assist you."
        )

    slot = appt.starts_at
    topic = appt.subject or subject
    kind = _arrival_kind(slot, clock)
    when = _fmt_time(slot)
    day = _fmt_date(slot)

    if kind == "imminent":
        return (
            f"Welcome, {visitor_name}! You're expected by {appt.host} "
            f"{when} for {topic}. Please take a seat in the waiting area."
        )
    if kind == "later_today":
        return (
            f"Hello {visitor_name}. Your appointment with {appt.host} is today "
            f"at {slot.strftime('%H:%M')} for {topic}. "
            f"You're a bit early — you may wait in the lounge or come back "
            f"closer to your slot."
        )
    if kind == "future":
        return (
            f"Hello {visitor_name}! I can confirm your appointment with {appt.host} "
            f"on {day} at {slot.strftime('%H:%M')} for {topic}. "
            f"That's not today — you're all set, no need to wait. See you then!"
        )
    return (
        f"{visitor_name}, I found your appointment with {appt.host} today at "
        f"{slot.strftime('%H:%M')} for {topic}. That time has already passed — "
        f"please speak with reception; they may still be able to help you."
    )


def book_appointment(
    visitor_name: str,
    host_name: str,
    appointment_at: datetime,
    staff_notes: str = "",
) -> str:
    """Create a NEW appointment in the calendar."""
    print("[Business] book_appointment(")
    print(f"    visitor_name={visitor_name!r},")
    print(f"    host_name={host_name!r},")
    print(f"    appointment_at={appointment_at!r},")
    print(f"    staff_notes={staff_notes!r},")
    print(")")

    if _find_exact(visitor_name, host_name, appointment_at):
        return (
            f"{visitor_name}, you already have an appointment with {host_name} "
            f"on {_fmt_time(appointment_at)}. Please proceed to check-in."
        )

    subject = staff_notes or "Walk-in"
    APPOINTMENTS.append(
        Appointment(visitor_name, host_name, appointment_at, subject)
    )
    return (
        f"All set, {visitor_name}! I've booked your appointment with {host_name} "
        f"on {_fmt_time(appointment_at)}."
    )


def cancel_visit(
    visitor_name: str,
    appointment_at: datetime,
    *,
    host_name: str = "",
    subject: str = "",
) -> str:
    print("[Business] cancel_visit(")
    print(f"    visitor_name={visitor_name!r},")
    print(f"    appointment_at={appointment_at!r},")
    print(f"    host_name={host_name!r},")
    print(")")

    target = next(
        (
            a
            for a in APPOINTMENTS
            if _same_name(a.visitor, visitor_name) and a.starts_at == appointment_at
        ),
        None,
    )
    if target is None:
        return (
            f"I'm sorry, I couldn't find an active appointment for {visitor_name} "
            f"to cancel. It may already have been cancelled."
        )

    APPOINTMENTS.remove(target)
    return (
        f"Done, {visitor_name}. Your appointment with {host_name or target.host} "
        f"on {_fmt_time(appointment_at)} has been cancelled. "
        f"We hope to see you again soon."
    )


def answer_visitor(direct_reply: str) -> str:
    print("[Business] answer_visitor(")
    print(f"    direct_reply={direct_reply!r},")
    print(")")
    return direct_reply


def execute_business_action(
    decision: FrontDeskDecision,
    strategy_result: Optional[StrategyResult] = None,
    *,
    now: Optional[datetime] = None,
) -> BusinessOutcome:
    """Dispatch to business logic; return desk reply and optional staff pager."""
    clock = now or datetime.now().astimezone()

    if decision.intent == "check_in":
        visitor, host, when, appt = _merged_fields(decision, strategy_result)
        if appt and (not host or not when):
            host = host or appt.host
            when = when or appt.starts_at
        if not _is_blank(visitor) and not _is_blank(host) and when:
            msg = check_in_visitor(
                visitor,
                host,
                when,
                now=clock,
                subject=appt.subject if appt else "",
            )
            if "already passed" in msg:
                return BusinessOutcome(
                    visitor_message=msg,
                    staff_requested=True,
                    staff_brief=(
                        f"Late check-in: {visitor} expected by {host} at "
                        f"{when.strftime('%H:%M')}; slot has passed. Desk asked them "
                        f"to wait for reception."
                    ),
                )
            if "cannot find" in msg:
                return BusinessOutcome(
                    visitor_message=msg,
                    staff_requested=True,
                    staff_brief=(
                        f"Check-in: {visitor} claims an appointment with {host} "
                        f"but nothing matches in the calendar."
                    ),
                )
            slot = appt.starts_at if appt else when
            kind = _arrival_kind(slot, clock)
            return BusinessOutcome(
                visitor_message=msg,
                notify_host=kind == "imminent",
                host_name=(appt.host if appt else host) or "",
                visitor_name=visitor or "",
                appointment_at=slot,
                appointment_subject=(appt.subject if appt else "") or "",
            )

        name = visitor or "there"
        return BusinessOutcome(
            visitor_message=(
                f"I'm sorry {name}, I couldn't verify your appointment in our system. "
                f"A staff member will assist you shortly."
            ),
            staff_requested=True,
            staff_brief=(
                f"Check-in incomplete: visitor {name!r}, host {host!r}, "
                f"time {when!r}. Calendar lookup did not resolve all fields."
            ),
        )

    if decision.intent == "book_appointment":
        visitor, host, when, _ = _merged_fields(decision, strategy_result)
        if not _is_blank(visitor) and not _is_blank(host) and when:
            return BusinessOutcome(
                visitor_message=book_appointment(
                    visitor, host, when, decision.staff_notes or ""
                )
            )
        return BusinessOutcome(
            visitor_message=(
                "I need a few more details to book your appointment. "
                "Please speak with reception."
            ),
            staff_requested=True,
            staff_brief=(
                f"Walk-in booking: missing details for {visitor or 'unknown visitor'} "
                f"(host={host!r}, time={when!r})."
            ),
        )

    if decision.intent == "cancel_appointment":
        appt = strategy_result.appointment_found if strategy_result else None
        if appt:
            return BusinessOutcome(
                visitor_message=cancel_visit(
                    appt.visitor,
                    appt.starts_at,
                    host_name=appt.host,
                    subject=appt.subject,
                )
            )
        name = decision.visitor_name or "there"
        return BusinessOutcome(
            visitor_message=(
                f"I'm sorry {name}, I couldn't locate the appointment to cancel. "
                f"Please confirm the name and date with reception."
            ),
            staff_requested=True,
            staff_brief=(
                f"Cancellation failed: {name}, cancel_when="
                f"{decision.cancel_date_text or decision.appointment_at!r}."
            ),
        )

    if decision.intent == "general_inquiry":
        if decision.direct_reply:
            return BusinessOutcome(visitor_message=answer_visitor(decision.direct_reply))
        return BusinessOutcome(
            visitor_message=(
                "I'm not sure I understood. Could you rephrase, or speak with reception?"
            ),
            staff_requested=True,
            staff_brief="General inquiry: desk could not answer; visitor needs a human.",
        )

    return BusinessOutcome(
        visitor_message="Sorry, something went wrong. Please speak with reception.",
        staff_requested=True,
        staff_brief=f"Unhandled intent {decision.intent!r}.",
    )
