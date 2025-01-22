# API Pokédex FastAPI

```bash
uvicorn main:app --reload
```

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
    "message": "Bienvenue dans le Pokédex de FastAPI!"
}
```

### 2. Liste des Pokémon

- **Méthode :** `GET`
- **URL :** `/pokemons/`
- **URL complète :** [http://localhost:8000/pokemons/](http://localhost:8000/pokemons/)
- **Paramètres facultatifs :**
  - `type` : Filtrer par type (ex : Electric)
  - `min_power` : Récupérer les Pokémon d'une puissance minimale
- **Exemples :**
  - Filtrer par type : [http://localhost:8000/pokemons/?type=Fire](http://localhost:8000/pokemons/?type=Fire)
  - Filtrer par puissance : [http://localhost:8000/pokemons/?min_power=50](http://localhost:8000/pokemons/?min_power=50)

### 3. Rechercher un Pokémon par ID

- **Méthode :** `GET`
- **URL :** `/pokemons/{pokemon_id}`
- **URL exemple :** [http://localhost:8000/pokemons/1](http://localhost:8000/pokemons/1)

### 4. Ajouter un nouveau Pokémon

- **Méthode :** `POST`
- **URL :** `/pokemons/`
- **URL complète :** [http://localhost:8000/pokemons/](http://localhost:8000/pokemons/)
- **Corps de la requête (JSON) :**
```json
{
    "id": 5,
    "name": "Eevee",
    "type": "Normal",
    "power": 50
}
```
- **Exemple avec curl :**
```bash
curl -X POST http://localhost:8000/pokemons/ \
     -H "Content-Type: application/json" \
     -d '{"id": 5, "name": "Eevee", "type": "Normal", "power": 50}'
```

### 5. Supprimer un Pokémon

- **Méthode :** `DELETE`
- **URL :** `/pokemons/{pokemon_id}`
- **URL exemple :** [http://localhost:8000/pokemons/2](http://localhost:8000/pokemons/2)
