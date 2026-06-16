"""Send staff pager alerts to a Discord channel (REST API, no running bot required)."""

from __future__ import annotations

import json
import urllib.error
import urllib.request

from .config import alert_channel_id, bot_token
from .host_mapping import mention_for_host


def _post_message(content: str) -> None:
    channel_id = alert_channel_id()
    url = f"https://discord.com/api/v10/channels/{channel_id}/messages"
    payload = json.dumps({"content": content}).encode("utf-8")
    request = urllib.request.Request(
        url,
        data=payload,
        method="POST",
        headers={
            "Authorization": f"Bot {bot_token()}",
            "Content-Type": "application/json",
            "User-Agent": "DiscordGuichet (BacASable demo, +https://github.com)",
        },
    )
    try:
        with urllib.request.urlopen(request, timeout=15) as response:
            if response.status >= 400:
                body = response.read().decode("utf-8", errors="replace")
                raise RuntimeError(f"Discord API HTTP {response.status}: {body}")
    except urllib.error.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"Discord API HTTP {exc.code}: {body}") from exc


def notify_staff(
    situation_brief: str,
    *,
    host_name: str | None = None,
    visitor_name: str | None = None,
    title: str = "**Pager guichet**",
) -> None:
    """Page reception on Discord when the desk cannot finish alone.

    Args:
        situation_brief: Same text as ``BusinessOutcome.staff_brief``.
        host_name: Agenda host to @mention when mapped in ``HOST_DISCORD_MAP``.
        visitor_name: Optional visitor name for the alert header.
        title: Bold heading line for the message.
    """
    mention = mention_for_host(host_name)
    lines = [title]
    if visitor_name:
        lines.append(f"Visiteur : **{visitor_name.strip()}**")
    if mention:
        lines.append(f"Hôte : {mention}")
    lines.append("")
    lines.append(situation_brief.strip())
    content = "\n".join(lines)

    if len(content) > 2000:
        content = content[:1997] + "..."

    _post_message(content)
    print(f"[Discord] Alert sent to channel {alert_channel_id()}")


def notify_check_in(
    visitor_name: str,
    host_name: str,
    *,
    appointment_at=None,
    subject: str = "",
) -> None:
    """Tell the host on Discord that their visitor has arrived (imminent slot)."""
    mention = mention_for_host(host_name)
    time_part = appointment_at.strftime("%H:%M") if appointment_at else "?"
    topic = subject.strip() or "rendez-vous"

    lines = ["**Visiteur arrivé**"]
    if mention:
        lines.append(f"{mention} — votre visiteur est à l'accueil.")
    else:
        lines.append(f"Hôte : **{host_name}** — visiteur à l'accueil.")
    lines.append(f"**{visitor_name.strip()}** — {topic} (créneau {time_part})")

    content = "\n".join(lines)
    _post_message(content)
    print(f"[Discord] Check-in alert sent to channel {alert_channel_id()}")
