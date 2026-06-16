#!/usr/bin/env python3
"""Send a test pager without calling the LLM."""

from __future__ import annotations

import argparse

from .notifier import notify_staff


def main() -> int:
    parser = argparse.ArgumentParser(description="Send a test Discord staff alert.")
    parser.add_argument(
        "--host",
        default="Julie",
        help="Agenda host name (must appear in HOST_DISCORD_MAP to get @mention).",
    )
    parser.add_argument(
        "--visitor",
        default="Thomas Martin",
        help="Visitor name shown in the alert header.",
    )
    parser.add_argument(
        "brief",
        nargs="?",
        default=(
            "Late check-in: Thomas Martin expected by Julie at 20:00; "
            "slot has passed. Desk asked them to wait for reception."
        ),
        help="Pager body (same shape as staff_brief).",
    )
    args = parser.parse_args()

    notify_staff(args.brief, host_name=args.host, visitor_name=args.visitor)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
