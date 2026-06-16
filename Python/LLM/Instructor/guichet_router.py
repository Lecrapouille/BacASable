"""
Front-desk routing via Instructor
=================================

The LLM reads what the visitor says and fills out a form in one go.
Then Python checks the calendar and replies. The LLM does not run the app.

We use the same flat schema for all providers (Gemini, Mistral, Groq, etc.).
We avoid Pydantic Unions. Gemini does not support them. Mistral often fails
with them too.
"""

from __future__ import annotations

from datetime import datetime
from typing import Literal, Optional

import instructor
from pydantic import BaseModel, Field


# -----------------------------------------------------------------------------
# What the LLM returns (one form, all providers)
# -----------------------------------------------------------------------------


class FrontDeskDecision(BaseModel):
    """The form the LLM fills in. Same boxes every time.

    ``intent`` says which case it is. Unused boxes stay ``None``.
    """

    intent: Literal[
        "check_in",
        "book_appointment",
        "cancel_appointment",
        "general_inquiry",
    ] = Field(
        description=(
            "What the visitor wants. Pick exactly one:\n"
            "- check_in: they already have a visit and are arriving or confirming it.\n"
            "- book_appointment: they want to schedule a new visit.\n"
            "- cancel_appointment: they want to cancel an existing visit.\n"
            "- general_inquiry: anything else (directions, restrooms, small talk)."
        )
    )
    visitor_name: Optional[str] = Field(
        None, description="Full visitor name (check_in / book appointment / cancel appointment)."
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


# -----------------------------------------------------------------------------
# LLM client
# -----------------------------------------------------------------------------

# Default model (Google AI Studio, free tier).
DEFAULT_MODEL = "google/gemini-2.5-flash"


def make_llm_client(*, model: str = DEFAULT_MODEL):
    """Connect to the LLM.

    Args:
        model: Provider and model name, e.g. ``google/gemini-2.5-flash``.
    """
    return instructor.from_provider(model)


# -----------------------------------------------------------------------------
# Clock
# -----------------------------------------------------------------------------

def time_context(now: Optional[datetime] = None) -> tuple[str, str]:
    """Build the date/time text sent to the LLM.

    Args:
        now: Fake "current" time for demos (given on the command line with ``--now``),
            else, uses real time if ``now`` is omitted.

    Returns:
        A readable line ("Today is…") and a fixed timestamp for "now".
    """
    dt = now or datetime.now().astimezone()
    human = f"Today is {dt.strftime('%A %d %B %Y')}, current time is {dt.strftime('%H:%M')}."
    iso = dt.replace(microsecond=0).isoformat()
    return human, iso


# -----------------------------------------------------------------------------
# Prompt and single LLM call
# -----------------------------------------------------------------------------


def system_prompt(ctx_human: str, now_iso: str) -> str:
    """Rules we send to the LLM before it fills the form.

    Args:
        ctx_human: Readable date/time line shown to the LLM ("Today is Monday…").
        now_iso: Fixed timestamp the LLM must use for "now" and relative times.

    Pick one intent. Turn "now" / "tomorrow" into a real time.
    If the visitor forgot something, leave the box blank and flag the app.
    """
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
        "   - cancel_appointment: visitor wants to cancel an existing visit.\n"
        "   - general_inquiry: anything else (directions, restrooms, small talk).\n"
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
    llm_client=None,
    llm_model: str = DEFAULT_MODEL,
    now: Optional[datetime] = None,
) -> FrontDeskDecision:
    """Turn the visitor's words into a filled ``FrontDeskDecision`` form.

    One LLM call only.

    Args:
        phrase: What the visitor said at the desk.
        llm_client: Instructor client. Built from ``model`` if not passed.
        llm_model: LLM to use when ``client`` is not passed.
        now: Fake (CLI ``--now``) or actual current time. Help to convert "now", "tomorrow", etc. to datetime.

    Returns:
        The filled ``FrontDeskDecision`` form.
    """
    client = llm_client or make_llm_client(model=llm_model)
    ctx_human, now_iso = time_context(now)

    return client.create(
        response_model=FrontDeskDecision,
        messages=[
            {"role": "system", "content": system_prompt(ctx_human, now_iso)},
            {"role": "user", "content": phrase},
        ],
    )


# -----------------------------------------------------------------------------
# Full flow: LLM → calendar → reply (optional staff call)
# -----------------------------------------------------------------------------


def handle_visitor_phrase(
    phrase: str,
    *,
    llm_client=None,
    llm_model: str = DEFAULT_MODEL,
    now: Optional[datetime] = None,
    simulate_staff: bool = False,
    discord: Optional[bool] = None,
    discord_staff: Optional[bool] = None,
) -> Optional[FrontDeskDecision]:
    """Run the full demo and print each step.

    Args:
        phrase: What the visitor said at the desk.
        llm_client: Instructor client. Built from ``llm_model`` if not passed.
        llm_model: LLM to use when ``llm_model`` is not passed.
        now: Fake (CLI ``--now``) or actual current time. Help to convert "now", "tomorrow", etc. to datetime.
        simulate_staff: If True and reception is needed, wait 3–5 s and LLM-simulate
            whether someone comes (CLI ``--simulate-staff``).
        discord: ``True`` / ``False`` / ``None`` (auto: on when ``DiscordGuichet/.env``
            is configured). ``discord_staff`` is a deprecated alias for ``discord=True``.

    Steps:
    1. Ask the LLM to fill the form
    2. Show what was extracted
    3. Look up the calendar if needed
    4. Run check-in / book / cancel / reply
    5. Optionally fake a call to reception

    Returns:
        The LLM form, or ``None`` on error.
    """
    ctx_human, _ = time_context(now)

    # --- What the visitor said, and what time we pretend it is ---
    print(f"\n[Visitor] {phrase!r}")
    print(f"[Context] {ctx_human}")
    print("-" * 50)

    # --- Step 1: ask the LLM ---
    try:
        decision = analyze_visitor_phrase(
            phrase,
            llm_client=llm_client,
            llm_model=llm_model,
            now=now
        )
    except Exception as exc:
        print(f"[Error] {exc}")
        return None

    # --- Step 2: show extracted fields (not raw JSON) ---
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

    # Import here so agenda_db and this file do not import each other at load time.
    from agenda_db import apply_strategies, execute_business_action, needs_strategy_resolution

    # --- Step 3: search the calendar (forgot host, time, cancel, etc.) ---
    strategy_result = None
    if needs_strategy_resolution(decision):
        print("[Escalate] Calendar lookup")
        if decision.resolution_hint:
            print(f"   hint: {decision.resolution_hint}")
        strategy_result = apply_strategies(decision, now=now)
        for line in strategy_result.log:
            print(f"   • {line}")

    # --- Step 4: check-in, book, cancel, or answer the question ---
    outcome = execute_business_action(decision, strategy_result, now=now)
    if outcome.visitor_message:
        print(f"\n[Desk → Visitor] {outcome.visitor_message}")

    # --- Step 5: Discord alerts and/or LLM staff simulation ---
    from discord_bridge import dispatch_discord_notifications, resolve_discord_enabled

    discord_flag = discord if discord is not None else discord_staff
    if resolve_discord_enabled(cli_flag=discord_flag):
        dispatch_discord_notifications(
            outcome,
            decision,
            strategy_result,
            enabled=True,
        )

    if simulate_staff and outcome.staff_requested and outcome.staff_brief:
        from staff_sim import run_staff_simulation

        client = llm_client or make_llm_client(model=llm_model)
        staff = run_staff_simulation(outcome.staff_brief, client=client)
        if staff:
            tag = "arrived" if staff.staff_arrived else "no-show"
            print(f"[Staff] Simulation ({tag})")
            print(f"\n[Desk → Visitor] {staff.message_to_visitor}")

    return decision
