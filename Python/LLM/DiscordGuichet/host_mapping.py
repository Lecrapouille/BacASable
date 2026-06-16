"""Resolve agenda host names to Discord mentions."""

from __future__ import annotations

from functools import lru_cache

from .config import host_discord_map, host_discord_map_json


@lru_cache(maxsize=1)
def _merged_map() -> dict[str, int]:
    merged = host_discord_map()
    merged.update(host_discord_map_json())
    return merged


def _lookup_host_id(host_name: str) -> int | None:
    mapping = _merged_map()
    if host_name in mapping:
        return mapping[host_name]

    folded = host_name.strip().casefold()
    for name, user_id in mapping.items():
        if name.strip().casefold() == folded:
            return user_id
    return None


def mention_for_host(host_name: str | None) -> str:
    """Return a Discord mention, or the plain host name if unmapped."""
    if not host_name or not host_name.strip():
        return ""
    user_id = _lookup_host_id(host_name)
    if user_id is None:
        return host_name.strip()
    return f"<@{user_id}>"
