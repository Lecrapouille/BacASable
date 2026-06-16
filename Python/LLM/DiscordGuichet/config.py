"""Load Discord settings from environment or a ``.env`` file."""

from __future__ import annotations

import json
import os
from pathlib import Path

from dotenv import load_dotenv

_ENV_LOADED = False


def _ensure_env_loaded() -> None:
    global _ENV_LOADED
    if _ENV_LOADED:
        return
    env_path = Path(__file__).resolve().parent / ".env"
    load_dotenv(env_path)
    _ENV_LOADED = True


def bot_token() -> str:
    _ensure_env_loaded()
    token = os.getenv("DISCORD_BOT_TOKEN", "").strip()
    if not token:
        raise RuntimeError(
            "DISCORD_BOT_TOKEN is missing. Copy .env.example to .env and set your bot token."
        )
    return token


def alert_channel_id() -> int:
    _ensure_env_loaded()
    raw = os.getenv("DISCORD_ALERT_CHANNEL_ID", "").strip()
    if not raw:
        raise RuntimeError(
            "DISCORD_ALERT_CHANNEL_ID is missing. Create #alertes-guichet and paste its ID."
        )
    try:
        return int(raw)
    except ValueError as exc:
        raise RuntimeError(
            f"DISCORD_ALERT_CHANNEL_ID must be numeric, got {raw!r}."
        ) from exc


def host_discord_map() -> dict[str, int]:
    """Map agenda host names to Discord user IDs."""
    _ensure_env_loaded()
    raw = os.getenv("HOST_DISCORD_MAP", "").strip()
    if not raw:
        return {}

    mapping: dict[str, int] = {}
    for chunk in raw.split(","):
        chunk = chunk.strip()
        if not chunk:
            continue
        if "=" not in chunk:
            continue
        name, user_id = chunk.split("=", 1)
        name = name.strip()
        user_id = user_id.strip()
        if not name or not user_id:
            continue
        try:
            mapping[name] = int(user_id)
        except ValueError:
            continue
    return mapping


def host_discord_map_json() -> dict[str, int]:
    """Optional JSON override, e.g. ``{"Julie": 123, "Mr Martin": 456}``."""
    _ensure_env_loaded()
    raw = os.getenv("HOST_DISCORD_MAP_JSON", "").strip()
    if not raw:
        return {}
    data = json.loads(raw)
    if not isinstance(data, dict):
        raise RuntimeError("HOST_DISCORD_MAP_JSON must be a JSON object.")
    return {str(k): int(v) for k, v in data.items()}
