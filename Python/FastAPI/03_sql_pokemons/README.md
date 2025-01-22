# API Pokédex FastAPI avec SQLite

```bash
uvicorn main:app --reload
```

## Base de données

Cette version de l'API utilise SQLite comme base de données pour stocker les Pokémon de manière persistante.
La base de données est automatiquement créée au premier lancement dans le fichier `pokedex.db`.

## Interface de test interactive

Une interface Swagger est disponible à l'URL : [http://localhost:8000/docs](http://localhost:8000/docs)
Cette interface permet de tester facilement toutes les routes de l'API sans outils supplémentaires.

## Endpoints disponibles

### 1. Page d'accueil

- **Méthode :** `GET`
- **URL :** `/`
- **URL complète :** [http://localhost:8000/](http://localhost:8000/)
- **Réponse :**
```json
{
    "message": "Bienvenue dans le Pokédex de FastAPI avec SQLite!"
}
```

### 2. Gestion de la base de données

#### 2.1 Initialisation de la base de données

- **Méthode :** `POST`
- **URL :** `/init-db/`
- **URL complète :** [http://localhost:8000/init-db/](http://localhost:8000/init-db/)
- **Description :** Initialise la base de données avec une liste de Pokémon par défaut
- **Exemple avec curl :**
```bash
curl -X POST http://localhost:8000/init-db/
```

#### 2.2 Vider la base de données

- **Méthode :** `POST`
- **URL :** `/clear-db/`
- **URL complète :** [http://localhost:8000/clear-db/](http://localhost:8000/clear-db/)
- **Description :** Supprime tous les Pokémon de la base de données
- **Exemple avec curl :**
```bash
curl -X POST http://localhost:8000/clear-db/
```

### 3. Liste des Pokémon

- **Méthode :** `GET`
- **URL :** `/pokemons/`
- **URL complète :** [http://localhost:8000/pokemons/](http://localhost:8000/pokemons/)
- **Paramètres facultatifs :**
  - `type` : Filtrer par type (ex : Electric)
  - `min_power` : Récupérer les Pokémon d'une puissance minimale
- **Exemples :**
  - Filtrer par type : [http://localhost:8000/pokemons/?type=Fire](http://localhost:8000/pokemons/?type=Fire)
  - Filtrer par puissance : [http://localhost:8000/pokemons/?min_power=50](http://localhost:8000/pokemons/?min_power=50)

### 4. Rechercher un Pokémon par ID

- **Méthode :** `GET`
- **URL :** `/pokemons/{pokemon_id}`
- **URL exemple :** [http://localhost:8000/pokemons/1](http://localhost:8000/pokemons/1)

### 5. Ajouter un nouveau Pokémon

- **Méthode :** `POST`
- **URL :** `/pokemons/`
- **URL complète :** [http://localhost:8000/pokemons/](http://localhost:8000/pokemons/)
- **Corps de la requête (JSON) :**
```json
{
    "name": "Eevee",
    "type": "Normal",
    "power": 50
}
```
- **Exemple avec curl :**
```bash
curl -X POST http://localhost:8000/pokemons/ \
     -H "Content-Type: application/json" \
     -d '{"name": "Eevee", "type": "Normal", "power": 50}'
```

### 6. Mettre à jour un Pokémon

- **Méthode :** `PUT`
- **URL :** `/pokemons/{pokemon_id}`
- **URL exemple :** [http://localhost:8000/pokemons/1](http://localhost:8000/pokemons/1)
- **Corps de la requête (JSON) :**
```json
{
    "power": 60
}
```
- **Exemple avec curl :**
```bash
curl -X PUT http://localhost:8000/pokemons/1 \
     -H "Content-Type: application/json" \
     -d '{"power": 60}'
```

### 7. Supprimer un Pokémon

- **Méthode :** `DELETE`
- **URL :** `/pokemons/{pokemon_id}`
- **URL exemple :** [http://localhost:8000/pokemons/1](http://localhost:8000/pokemons/1)
- **Exemple avec curl :**
```bash
curl -X DELETE http://localhost:8000/pokemons/1
```

### 8. Afficher la fiche d'un Pokémon

- **Méthode :** `GET`
- **URL :** `/pokemons/{pokemon_id}/card`
- **URL exemple :** [http://localhost:8000/pokemons/1/card](http://localhost:8000/pokemons/1/card)
- **Description :** Affiche une fiche détaillée du Pokémon au format texte

## Pokémon par défaut

La base de données peut être initialisée avec les Pokémon suivants :
- Pikachu (Electric, 55)
- Bulbasaur (Grass, 49)
- Charmander (Fire, 52)
- Squirtle (Water, 48)
- Mewtwo (Psychic, 110)
- Dragonite (Dragon, 100)
- Snorlax (Normal, 85)
- Gengar (Ghost, 65)

Pour initialiser la base avec ces Pokémon, utilisez l'endpoint `/init-db/` :

```bash
# Initialisation de la base de données
curl -X POST http://localhost:8000/init-db/

# Vérification du résultat
curl http://localhost:8000/pokemons/
```

Le premier appel initialisera la base de données, et le second vous permettra de vérifier que les Pokémon ont bien été ajoutés.

## Exemples d'utilisation

### Réinitialisation complète de la base de données

```bash
# Vider la base de données
curl -X POST http://localhost:8000/clear-db/

# Initialiser avec les Pokémon par défaut
curl -X POST http://localhost:8000/init-db/

# Vérifier le résultat
curl http://localhost:8000/pokemons/
```

Le premier appel videra la base de données, le second l'initialisera avec les Pokémon par défaut, et le troisième vous permettra de vérifier que les Pokémon ont bien été ajoutés. 
