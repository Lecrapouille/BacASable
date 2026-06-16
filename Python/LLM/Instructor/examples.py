#!/usr/bin/env python3
"""
Front-desk demo with Instructor (flat schema, all providers).

    cd BacASable/Python/LLM/Instructor
    python3 -m venv .venv
    . .venv/bin/activate
    pip install -r requirements.txt
    export GOOGLE_API_KEY=...
    python examples.py

    python examples.py --phrase "Hi, I'm John Smith, here for my appointment now."
    python examples.py --model mistral/mistral-small-latest
    python examples.py --now "2026-06-16 21:30" --phrase "Hi, I'm Thomas Martin..."
    python examples.py --simulate-staff --phrase "Hello, I'm Robert Foo, here to check in."

Free-tier providers:
    --model google/gemini-2.5-flash
    --model groq/llama-3.3-70b-versatile
    --model mistral/mistral-small-latest
"""

from __future__ import annotations

import argparse
import sys
from datetime import datetime

from guichet_router import DEFAULT_MODEL, handle_visitor_phrase, make_client
from agenda_db import reset_calendar

DEMO_PHRASES = [
    (
        "Case 1 — check-in now (existing appointment)",
        "Hi, I'm Thomas Martin. I'm here for my appointment right now with Julie.",
    ),
    (
        "Case 2 — check-in, forgot host (calendar lookup)",
        "I'd like to check in please, my name is Sarah Bernard. "
        "But I completely forgot who I'm supposed to meet...",
    ),
    (
        "Case 3 — cancellation",
        "Oh no, something came up — I need to cancel my visit tomorrow. "
        "It's under Marc Durant.",
    ),
    (
        "Case 4 — off-topic",
        "Excuse me, could you tell me where the restrooms are?",
    ),
    (
        "Case 5 — check-in for tomorrow's appointment",
        "Hello, Marie Dubois. I have an appointment tomorrow at 2:30 pm with Dr Lemoine.",
    ),
    (
        "Case 6 — check-in, forgot time (calendar lookup, like case 2)",
        "Hi, I'm Léa Petit. I have a meeting with Mr Martin but I forgot what time.",
    ),
    (
        "Case 7 — check-in, unknown visitor (like case 2, not in calendar)",
        "Hello, I'm Robert Foo, here to check in for my appointment.",
    ),
    (
        "Case 8 — double cancellation (Marc Durant again, already removed in case 3)",
        "Actually I need to cancel Marc Durant's appointment for tomorrow, please.",
    ),
    (
        "Case 9 — book NEW appointment (walk-in, memoized for case 10)",
        "Hi, I'm Alice Wonder. I'd like to book an appointment with Mr Bob for the audit today.",
    ),
    (
        "Case 10 — cancel the visit just registered for Alice Wonder",
        "Sorry, please cancel Alice Wonder's visit, the audit is off.",
    ),
]


def parse_simulated_now(value: str) -> datetime:
    """Parse CLI ``--now`` as local wall-clock time."""
    tz = datetime.now().astimezone().tzinfo
    for fmt in ("%Y-%m-%d %H:%M:%S", "%Y-%m-%d %H:%M", "%Y-%m-%dT%H:%M:%S", "%Y-%m-%dT%H:%M"):
        try:
            naive = datetime.strptime(value, fmt)
            return naive.replace(tzinfo=tz)
        except ValueError:
            continue
    raise argparse.ArgumentTypeError(
        f"Invalid datetime {value!r}. Use e.g. 2026-06-16 21:30"
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Front desk — structured routing via Instructor (flat schema).",
    )
    parser.add_argument(
        "--phrase",
        action="append",
        dest="phrases",
        help="Visitor phrase to analyze (repeatable). Default: demo scenarios.",
    )
    parser.add_argument(
        "--model",
        default=DEFAULT_MODEL,
        help=f"Instructor model (default: {DEFAULT_MODEL}).",
    )
    parser.add_argument(
        "--now",
        type=parse_simulated_now,
        metavar="DATETIME",
        help='Simulate current date/time (e.g. "2026-06-16 21:30"). Seeds calendar too.',
    )
    parser.add_argument(
        "--simulate-staff",
        action="store_true",
        help="When desk pages reception, wait 3–5 s and LLM-simulate staff arrival.",
    )
    args = parser.parse_args()

    reset_calendar(now=args.now)

    try:
        client = make_client(model=args.model)
    except Exception as exc:
        print(f"[Error] {exc}", file=sys.stderr)
        return 1

    scenarios = (
        [(f"Phrase {i + 1}", p) for i, p in enumerate(args.phrases)]
        if args.phrases
        else DEMO_PHRASES
    )

    for title, phrase in scenarios:
        print(f"\n{'=' * 60}\n{title}\n{'=' * 60}")
        handle_visitor_phrase(
            phrase,
            client=client,
            model=args.model,
            now=args.now,
            simulate_staff=args.simulate_staff,
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
