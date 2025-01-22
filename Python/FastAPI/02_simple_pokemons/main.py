from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import List, Optional

app = FastAPI()

# Une base de données simulée
pokemon_db = [
    {"id": 1, "name": "Pikachu", "type": "Electric", "power": 55},
    {"id": 2, "name": "Charmander", "type": "Fire", "power": 39},
    {"id": 3, "name": "Bulbasaur", "type": "Grass", "power": 45},
    {"id": 4, "name": "Squirtle", "type": "Water", "power": 44},
    {"id": 5, "name": "Charizard", "type": "Fire", "power": 84},
    {"id": 6, "name": "Arcanine", "type": "Fire", "power": 90},
    {"id": 7, "name": "Flareon", "type": "Fire", "power": 65},
    {"id": 8, "name": "Magmar", "type": "Fire", "power": 75}
]

# Modèle de données pour valider les entrées
class Pokemon(BaseModel):
    id: int
    name: str
    type: str
    power: int

@app.get("/")
def welcome():
    return {"message": "Bienvenue dans le Pokédex de FastAPI!"}

@app.get("/pokemons/", response_model=List[Pokemon])
def get_all_pokemons(type: Optional[str] = None, min_power: Optional[int] = None):
    """
    Récupère tous les Pokémon, avec des filtres optionnels :
    - `type` pour filtrer par type (ex: Fire, Water).
    - `min_power` pour récupérer ceux ayant une puissance minimale.
    """
    results = pokemon_db
    if type:
        results = [p for p in results if p["type"].lower() == type.lower()]
    if min_power:
        results = [p for p in results if p["power"] >= min_power]
    return results

@app.get("/pokemons/{pokemon_id}", response_model=Pokemon)
def get_pokemon(pokemon_id: int):
    """
    Récupère un Pokémon par son ID.
    """
    for pokemon in pokemon_db:
        if pokemon["id"] == pokemon_id:
            return pokemon
    raise HTTPException(status_code=404, detail="Pokemon not found")

@app.post("/pokemons/", response_model=Pokemon)
def add_pokemon(pokemon: Pokemon):
    """
    Ajoute un nouveau Pokémon dans la base.
    """
    for existing in pokemon_db:
        if existing["id"] == pokemon.id:
            raise HTTPException(status_code=400, detail="Pokemon ID already exists")
    pokemon_db.append(pokemon.dict())
    return pokemon

@app.delete("/pokemons/{pokemon_id}", response_model=dict)
def delete_pokemon(pokemon_id: int):
    """
    Supprime un Pokémon par son ID.
    """
    global pokemon_db
    pokemon_db = [p for p in pokemon_db if p["id"] != pokemon_id]
    return {"message": f"Pokemon avec ID {pokemon_id} supprimé."}
