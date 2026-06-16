#!/usr/bin/env python3
"""
Minimal Discord bot — confirms the token works and listens on #alertes-guichet.

Run alongside Instructor with ``--discord-staff`` (notifier uses REST; this bot
is optional but useful to verify permissions and react to staff replies).

    cd BacASable/Python/LLM/DiscordGuichet
    python3 -m venv .venv && . .venv/bin/activate
    pip install -r requirements.txt
    cp .env.example .env   # fill token + channel id
    python run_bot.py      # from DiscordGuichet/ (recommended)
    # or: cd .. && python -m DiscordGuichet.bot   # from Python/LLM/
"""

from __future__ import annotations

import discord

from .config import alert_channel_id, bot_token


class GuichetBot(discord.Client):
    def __init__(self) -> None:
        intents = discord.Intents.default()
        intents.message_content = True
        super().__init__(intents=intents)
        self._alert_channel_id = alert_channel_id()

    async def on_ready(self) -> None:
        print(f"[Discord] Logged in as {self.user} (id={self.user.id})")
        try:
            channel = self.get_channel(self._alert_channel_id)
            if channel is None:
                channel = await self.fetch_channel(self._alert_channel_id)
        except discord.Forbidden:
            print(
                f"[Discord] Cannot access channel {self._alert_channel_id}. "
                "Check DISCORD_ALERT_CHANNEL_ID and that the bot is on the server "
                "with View Channel + Send Messages on #alertes-guichet."
            )
            return
        except discord.NotFound:
            print(
                f"[Discord] Channel {self._alert_channel_id} not found. "
                "Check DISCORD_ALERT_CHANNEL_ID in .env."
            )
            return

        print(f"[Discord] Alert channel: #{channel.name} ({channel.id})")
        await channel.send(
            "Bot guichet en ligne. Les alertes staff arrivent ici "
            "(lancez Instructor avec ``--discord-staff``)."
        )

    async def on_message(self, message: discord.Message) -> None:
        if message.author.bot:
            return
        if message.channel.id != self._alert_channel_id:
            return
        if message.content.strip().lower() in {"ping", "!ping"}:
            await message.reply("Guichet OK — prêt à recevoir les pagers.")


def main() -> None:
    client = GuichetBot()
    client.run(bot_token())


if __name__ == "__main__":
    main()
