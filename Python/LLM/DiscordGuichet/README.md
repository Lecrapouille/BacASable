# Discord guichet — pager staff

Envoie sur Discord les alertes ``staff_brief`` produites par [Instructor](../Instructor/) quand le guichet ne peut pas terminer seul.

## Prérequis Discord

1. Créer un **serveur de test** (gratuit).
2. [Developer Portal](https://discord.com/developers/applications) → New Application → **Bot** → copier le token.
3. OAuth2 → URL Generator → scope `bot`, permissions **View Channel**, **Send Messages**, **Read Message History** → inviter le bot sur votre serveur.
4. Créer le salon `#alertes-guichet`, activer le **mode développeur** (Paramètres → Avancés), copier l’ID du salon (clic droit **sur le salon**, pas sur le serveur).

## Installation

```bash
cd BacASable/Python/LLM/DiscordGuichet
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
    cp .env.example .env
    # éditer .env : token, channel id, mapping hôtes
    python run_bot.py          # depuis ce dossier (recommandé)
```

> **Important :** `python -m DiscordGuichet.bot` ne fonctionne **que** si le répertoire
> courant est ``Python/LLM`` (parent de ``DiscordGuichet/``). Depuis
> ``DiscordGuichet/``, utilisez ``python run_bot.py`` ou ``python run_demo_notify.py``.

## Tester sans LLM

Depuis ``DiscordGuichet/`` (venv activé) :

```bash
python run_demo_notify.py
python run_demo_notify.py --host Julie --visitor "Robert Foo" \
  "Check-in: unknown visitor Robert Foo."
```

Ou depuis ``Python/LLM`` :

```bash
python -m DiscordGuichet.demo_notify
```

## Bot optionnel (vérif connexion)

Depuis ``DiscordGuichet/`` :

```bash
python run_bot.py
```

Ou depuis ``Python/LLM`` :

```bash
python -m DiscordGuichet.bot
```

Le bot poste un message de démarrage dans `#alertes-guichet` et répond `ping`.

## Brancher Instructor

Configure ``../DiscordGuichet/.env``, puis depuis Instructor :

```bash
cd ../Instructor
pip install -r requirements.txt
python examples.py --phrase "Hello, I'm Robert Foo, here to check in."
python examples.py --now "2026-06-16 20:05" \
  --phrase "Hi, I'm Thomas Martin. I'm here for my appointment right now with Julie."
```

Discord s'active **automatiquement** si ``.env`` est valide. ``--discord`` force l'envoi ; ``--no-discord`` le désactive.

## Dépannage « Missing Access » (403)

1. **Réinviter le bot** avec **View Channel** coché (sans ça, le bot ne voit pas le salon).
2. Sur le serveur : Paramètres du salon `#alertes-guichet` → Permissions → rôle du bot → autoriser **Voir le salon** + **Envoyer des messages**.
3. Vérifier l’ID : clic droit sur `#alertes-guichet` → Copier l’identifiant du salon (pas l’ID du serveur ni votre ID utilisateur).
4. Le bot doit être **membre du même serveur** que le salon référencé dans `.env`.

## Fichiers

| Fichier | Rôle |
|---------|------|
| `notifier.py` | Envoi REST vers le salon (pas besoin de `bot.py` pour pager) |
| `bot.py` | Bot minimal pour tester token / permissions |
| `host_mapping.py` | Nom d’hôte agenda → mention Discord |
| `config.py` | Variables d’environnement |
| `demo_notify.py` | Alert de test |
