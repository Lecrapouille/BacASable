"""``python -m DiscordGuichet`` — lancer depuis ``Python/LLM``, pas depuis ce dossier."""

from __future__ import annotations

import sys
from pathlib import Path

if __name__ == "__main__":
    here = Path(__file__).resolve().parent
    llm_root = here.parent
    if str(llm_root) not in sys.path:
        sys.path.insert(0, str(llm_root))

    from DiscordGuichet.bot import main

    main()
