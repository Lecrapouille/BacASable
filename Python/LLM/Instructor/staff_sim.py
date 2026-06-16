"""
Async simulation of a human staff member answering a desk pager.
================================================================

After the automated desk tells the visitor to wait, this module sleeps 3–5 s
(random) then asks the LLM whether someone from reception actually shows up.
"""

from __future__ import annotations

import asyncio
import random
from typing import Optional

from pydantic import BaseModel, Field


class StaffArrivalOutcome(BaseModel):
    """LLM simulation: did staff arrive, and what do they tell the visitor?"""

    staff_arrived: bool = Field(
        description="True if a reception staff member came to the desk."
    )
    message_to_visitor: str = Field(
        description=(
            "One or two sentences the desk says to the visitor AFTER the wait. "
            "If staff_arrived=False, explain nobody is free yet or ask to keep waiting."
        )
    )


def _staff_system_prompt() -> str:
    return (
        "You simulate whether a human reception staff member responds to a desk pager. "
        "Given the situation brief, decide realistically:\n"
        "- staff_arrived=True (~60%): someone comes (receptionist, host, colleague).\n"
        "- staff_arrived=False (~40%): everyone is busy in a meeting / at lunch.\n"
        "Write message_to_visitor as a complete polite sentence for the visitor."
    )


async def simulate_staff_call(
    situation_brief: str,
    *,
    client,
) -> StaffArrivalOutcome:
    """Wait 3–5 s, then LLM decides if staff arrives."""
    delay = random.uniform(3.0, 5.0)
    print(f"[Staff] Paging reception… (~{delay:.1f}s)")
    await asyncio.sleep(delay)

    return client.create(
        response_model=StaffArrivalOutcome,
        messages=[
            {"role": "system", "content": _staff_system_prompt()},
            {
                "role": "user",
                "content": (
                    f"Situation at the front desk:\n{situation_brief}\n\n"
                    "Simulate staff arrival and the follow-up sentence."
                ),
            },
        ],
    )


def run_staff_simulation(
    situation_brief: str,
    *,
    client,
) -> Optional[StaffArrivalOutcome]:
    """Sync wrapper for CLI scripts (``asyncio.run``)."""
    try:
        return asyncio.run(simulate_staff_call(situation_brief, client=client))
    except Exception as exc:
        print(f"[Staff] Simulation failed: {exc}")
        return None
