"""
Front-desk routing via Instructor
=================================

One-shot flow: the LLM fills a flat Pydantic model, then Python dispatches rigidly
(your code runs the business logic — not the model).

We use a **flat schema for every provider** (Gemini, Mistral, Groq…). Pydantic
Unions are avoided: Gemini rejects them, and Mistral often raises validation
errors with discriminated unions in practice.
"""

from __future__ import annotations

from datetime import datetime
from typing import Literal, Optional

import instructor
from pydantic import BaseModel, Field


# -----------------------------------------------------------------------------
# LLM response model — flat schema, all providers
# -----------------------------------------------------------------------------


class FrontDeskDecision(BaseModel):
    """Single flat schema passed to Instructor as ``response_model``.

    Fill only the fields relevant to ``intent``. Leave others as None.
    """

    intent: Literal[
        "check_in",
        "book_appointment",
        "cancel_appointment",
        "general_inquiry",
    ] = Field(
        description=(
            "Main visitor intent. "
            "check_in = arriving for an EXISTING appointment "
            "('check in', 'I'm here for my appointment'). "
            "book_appointment = schedule a NEW visit."
        )
    )
    visitor_name: Optional[str] = Field(
        None, description="Full visitor name (check_in / book / cancel flows)."
    )
    host_name: Optional[str] = Field(
        None,
        description=(
            "Host employee. None ONLY if the visitor forgot — never invent."
        ),
    )
    appointment_at: Optional[datetime] = Field(
        None,
        description=(
            "Absolute ISO 8601 datetime. Resolve 'now', 'tomorrow 2pm', etc. "
            "using now_iso from the system prompt."
        ),
    )
    staff_notes: Optional[str] = Field(
        None, description="One short note for staff."
    )
    cancel_date_text: Optional[str] = Field(
        None, description="Free-text cancel date (cancel flow)."
    )
    direct_reply: Optional[str] = Field(
        None,
        description=(
            "Polite reply to say OUT LOUD to the visitor (general inquiry). "
            "Must be a complete sentence."
        ),
    )

    needs_app_assistance: bool = Field(
        False,
        description=(
            "True when the app must look up the calendar (forgot host/time). "
            "Set when the visitor explicitly forgot something."
        ),
    )
    resolution_hint: Optional[str] = Field(
        None,
        description=(
            "If needs_app_assistance=True: one sentence for staff logs. "
            "NEVER 'ask the visitor again'."
        ),
    )


DEFAULT_MODEL = "google/gemini-2.5-flash"


def make_client(*, model: str = DEFAULT_MODEL):
    """Build an Instructor client (provider inferred from ``provider/model``)."""
    return instructor.from_provider(model)


def time_context(now: Optional[datetime] = None) -> tuple[str, str]:
    """Return (human-readable, iso8601) for relative-time resolution."""
    dt = now or datetime.now().astimezone()
    human = f"Today is {dt.strftime('%A %d %B %Y')}, current time is {dt.strftime('%H:%M')}."
    iso = dt.replace(microsecond=0).isoformat()
    return human, iso


def _system_prompt(ctx_human: str, now_iso: str) -> str:
    return (
        "You are a front-desk routing brain. Analyze the visitor's sentence "
        "and fill the FrontDeskDecision schema.\n\n"
        f"Time context: {ctx_human}\n"
        f"now_iso = {now_iso}  (use this ISO 8601 value as-is)\n\n"
        "Strict rules:\n"
        "1. Pick one intent:\n"
        "   - check_in: visitor arrives for an EXISTING appointment "
        "('check in', 'I'm here', 'I have an appointment'). "
        "If they only want to CONFIRM a future slot (e.g. 'tomorrow at 2pm'), "
        "still use check_in — the app will reply accordingly.\n"
        "   - book_appointment: visitor wants to SCHEDULE a new visit.\n"
        "   - cancel_appointment / general_inquiry.\n"
        "2. TIME RESOLUTION — convert relative phrases to appointment_at ISO 8601:\n"
        "   - 'now', 'right away' → now_iso\n"
        "   - 'tomorrow 2pm' → tomorrow at 14:00 local\n"
        "   Leave appointment_at None ONLY if the visitor forgot the time "
        "or gave no time hint.\n"
        "3. MISSING INFO — if the visitor forgot host or time:\n"
        "   - leave that field None, set needs_app_assistance=True.\n"
        "   NEVER ask the visitor again for what they forgot.\n"
        "4. general_inquiry: fill direct_reply with a complete polite sentence "
        "to say to the visitor (e.g. directions to restrooms).\n"
        "5. Do NOT invent names, dates, or hosts.\n"
    )


def analyze_visitor_phrase(
    phrase: str,
    *,
    client=None,
    model: str = DEFAULT_MODEL,
    now: Optional[datetime] = None,
) -> FrontDeskDecision:
    """Free-text visitor phrase → validated FrontDeskDecision."""
    c = client or make_client(model=model)
    ctx_human, now_iso = time_context(now)

    return c.create(
        response_model=FrontDeskDecision,
        messages=[
            {"role": "system", "content": _system_prompt(ctx_human, now_iso)},
            {"role": "user", "content": phrase},
        ],
    )


def handle_visitor_phrase(
    phrase: str,
    *,
    client=None,
    model: str = DEFAULT_MODEL,
    now: Optional[datetime] = None,
    simulate_staff: bool = False,
) -> Optional[FrontDeskDecision]:
    """Analyze, run business logic, speak to the visitor."""
    ctx_human, _ = time_context(now)

    print(f"\n[Visitor] {phrase!r}")
    print(f"[Context] {ctx_human}")
    print("-" * 50)

    try:
        decision = analyze_visitor_phrase(
            phrase, client=client, model=model, now=now
        )
    except Exception as exc:
        print(f"[Error] {exc}")
        return None

    # Staff-facing summary (structured fields extracted by the LLM).
    print("[Staff] Extracted intent and fields:")
    print(f"   intent         : {decision.intent}")
    print(f"   visitor_name   : {decision.visitor_name or '(not set)'}")
    print(f"   host_name      : {decision.host_name or '(not set)'}")
    print(f"   appointment_at : {decision.appointment_at or '(not set)'}")
    if decision.cancel_date_text:
        print(f"   cancel_when    : {decision.cancel_date_text}")
    if decision.staff_notes:
        print(f"   staff_notes    : {decision.staff_notes}")
    if decision.direct_reply:
        print(f"   direct_reply   : {decision.direct_reply}")

    from agenda_db import apply_strategies, execute_business_action, needs_strategy_resolution

    strategy_result = None
    if needs_strategy_resolution(decision):
        print("[Escalate] Calendar lookup")
        if decision.resolution_hint:
            print(f"   hint: {decision.resolution_hint}")
        strategy_result = apply_strategies(decision, now=now)
        for line in strategy_result.log:
            print(f"   • {line}")

    outcome = execute_business_action(decision, strategy_result, now=now)
    if outcome.visitor_message:
        print(f"\n[Desk → Visitor] {outcome.visitor_message}")

    if simulate_staff and outcome.staff_requested and outcome.staff_brief:
        from staff_sim import run_staff_simulation

        c = client or make_client(model=model)
        staff = run_staff_simulation(outcome.staff_brief, client=c)
        if staff:
            tag = "arrived" if staff.staff_arrived else "no-show"
            print(f"[Staff] Simulation ({tag})")
            print(f"\n[Desk → Visitor] {staff.message_to_visitor}")

    return decision
