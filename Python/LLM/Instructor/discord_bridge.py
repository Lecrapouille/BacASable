"""Bridge Instructor → DiscordGuichet (pager + host check-in alerts)."""

from __future__ import annotations

import sys
from pathlib import Path
from typing import TYPE_CHECKING, Optional

if TYPE_CHECKING:
    from agenda_db import BusinessOutcome, StrategyResult
    from guichet_router import FrontDeskDecision

_LLM_ROOT = Path(__file__).resolve().parent.parent


def _ensure_discord_guichet() -> None:
    root = str(_LLM_ROOT)
    if root not in sys.path:
        sys.path.insert(0, root)


def discord_configured() -> bool:
    """True when DiscordGuichet/.env (or env vars) has token + channel id."""
    _ensure_discord_guichet()
    try:
        from DiscordGuichet.config import alert_channel_id, bot_token

        bot_token()
        alert_channel_id()
        return True
    except RuntimeError:
        return False


def resolve_discord_enabled(*, cli_flag: Optional[bool] = None) -> bool:
    """None = auto-detect from config; True/False = force on/off."""
    if cli_flag is False:
        return False
    if cli_flag is True:
        return True
    return discord_configured()


def resolve_host_name(
    decision: FrontDeskDecision,
    strategy_result: Optional[StrategyResult],
) -> Optional[str]:
    if strategy_result is not None:
        if strategy_result.host_name:
            return strategy_result.host_name
        appt = strategy_result.appointment_found
        if appt is not None:
            return appt.host
    if decision.host_name:
        return decision.host_name
    return None


def dispatch_discord_notifications(
    outcome: BusinessOutcome,
    decision: FrontDeskDecision,
    strategy_result: Optional[StrategyResult],
    *,
    enabled: bool,
) -> None:
    """Send Discord alerts for staff pager and/or imminent check-in."""
    if not enabled:
        return

    _ensure_discord_guichet()
    try:
        from DiscordGuichet import notify_check_in, notify_staff
    except ImportError as exc:
        print(f"[Discord] DiscordGuichet not available: {exc}")
        return

    if outcome.staff_requested and outcome.staff_brief:
        try:
            notify_staff(
                outcome.staff_brief,
                host_name=resolve_host_name(decision, strategy_result),
                visitor_name=decision.visitor_name,
            )
        except Exception as exc:
            print(f"[Discord] Pager failed: {exc}")
        return

    if outcome.notify_host and outcome.host_name and outcome.visitor_name:
        try:
            notify_check_in(
                outcome.visitor_name,
                outcome.host_name,
                appointment_at=outcome.appointment_at,
                subject=outcome.appointment_subject,
            )
        except Exception as exc:
            print(f"[Discord] Check-in alert failed: {exc}")
