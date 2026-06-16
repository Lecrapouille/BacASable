#!/usr/bin/env python3
"""Envoyer une alerte test depuis ``DiscordGuichet/`` (où se trouve le venv)."""

from __future__ import annotations

import sys
from pathlib import Path

_LLM_ROOT = Path(__file__).resolve().parent.parent
if str(_LLM_ROOT) not in sys.path:
    sys.path.insert(0, str(_LLM_ROOT))

from DiscordGuichet.demo_notify import main

if __name__ == "__main__":
    raise SystemExit(main())
