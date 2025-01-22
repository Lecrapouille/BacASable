# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief FastAPI Pokemon API with SQLite database
# /// @details This module implements a REST API for managing Pokemon data using FastAPI and SQLite
# //////////////////////////////////////////////////////////////////////////////////////////////////

from fastapi import FastAPI, HTTPException, Depends
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from pydantic import BaseModel
from typing import List, Optional
from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session

# Create FastAPI application
app = FastAPI()

# Mount static files directory
app.mount("/static", StaticFiles(directory="static"), name="static")

# Serve index.html at root URL
@app.get("/", response_class=FileResponse)
def read_root():
    return "static/index.html"

# SQLite database configuration
DATABASE_URL = "sqlite:///./pokedex.db"
engine = create_engine(DATABASE_URL, connect_args={"check_same_thread": False})
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Default pokemon data for database initialization
# //////////////////////////////////////////////////////////////////////////////////////////////////
DEFAULT_POKEMONS = [
    {"name": "Pikachu", "type": "Electric", "power": 55},
    {"name": "Bulbasaur", "type": "Grass", "power": 49},
    {"name": "Charmander", "type": "Fire", "power": 52},
    {"name": "Squirtle", "type": "Water", "power": 48},
    {"name": "Mewtwo", "type": "Psychic", "power": 110},
    {"name": "Dragonite", "type": "Dragon", "power": 100},
    {"name": "Snorlax", "type": "Normal", "power": 85},
    {"name": "Gengar", "type": "Ghost", "power": 65},
]

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief SQLAlchemy model for Pokemon
# //////////////////////////////////////////////////////////////////////////////////////////////////
class Pokemon(Base):
    __tablename__ = "pokemons"
    id = Column(Integer, primary_key=True, index=True)
    name = Column(String, index=True)
    type = Column(String)
    power = Column(Integer)

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Pydantic models for request/response data validation
# //////////////////////////////////////////////////////////////////////////////////////////////////
class PokemonCreate(BaseModel):
    name: str
    type: str
    power: int

class PokemonUpdate(BaseModel):
    name: Optional[str] = None
    type: Optional[str] = None
    power: Optional[int] = None

class PokemonResponse(BaseModel):
    id: int
    name: str
    type: str
    power: int

    class Config:
        orm_mode = True

# Create database tables
Base.metadata.create_all(bind=engine)

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Database session dependency
# /// @return Database session
# //////////////////////////////////////////////////////////////////////////////////////////////////
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

# API Routes

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Welcome endpoint
# /// @return Welcome message
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.get("/")
def welcome():
    return {"message": "Bienvenue dans le Pokédex de FastAPI avec SQLite!"}

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Get all pokemons with optional filtering
# /// @param[in] type Optional pokemon type filter
# /// @param[in] min_power Optional minimum power filter
# /// @param[in] db Database session
# /// @return List of pokemon matching the filters
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.get("/pokemons/", response_model=List[PokemonResponse])
def get_all_pokemons(
    type: Optional[str] = None,
    min_power: Optional[int] = None,
    db: Session = Depends(get_db)
):
    query = db.query(Pokemon)
    if type:
        query = query.filter(Pokemon.type == type)
    if min_power:
        query = query.filter(Pokemon.power >= min_power)
    return query.all()

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Get a specific pokemon by ID
# /// @param[in] pokemon_id ID of the pokemon to retrieve
# /// @param[in] db Database session
# /// @return Pokemon data if found
# /// @throw HTTPException if pokemon not found
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.get("/pokemons/{pokemon_id}", response_model=PokemonResponse)
def get_pokemon(pokemon_id: int, db: Session = Depends(get_db)):
    pokemon = db.query(Pokemon).filter(Pokemon.id == pokemon_id).first()
    if not pokemon:
        raise HTTPException(status_code=404, detail="Pokemon not found")
    return pokemon

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Create a new pokemon
# /// @param[in] pokemon Pokemon data to create
# /// @param[in] db Database session
# /// @return Created pokemon data
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.post("/pokemons/", response_model=PokemonResponse)
def add_pokemon(pokemon: PokemonCreate, db: Session = Depends(get_db)):
    db_pokemon = Pokemon(name=pokemon.name, type=pokemon.type, power=pokemon.power)
    db.add(db_pokemon)
    db.commit()
    db.refresh(db_pokemon)
    return db_pokemon

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Update an existing pokemon
# /// @param[in] pokemon_id ID of the pokemon to update
# /// @param[in] update_data New pokemon data
# /// @param[in] db Database session
# /// @return Updated pokemon data
# /// @throw HTTPException if pokemon not found
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.put("/pokemons/{pokemon_id}", response_model=PokemonResponse)
def update_pokemon(
    pokemon_id: int,
    update_data: PokemonUpdate,
    db: Session = Depends(get_db)
):
    pokemon = db.query(Pokemon).filter(Pokemon.id == pokemon_id).first()
    if not pokemon:
        raise HTTPException(status_code=404, detail="Pokemon not found")
    
    if update_data.name:
        pokemon.name = update_data.name
    if update_data.type:
        pokemon.type = update_data.type
    if update_data.power:
        pokemon.power = update_data.power

    db.commit()
    db.refresh(pokemon)
    return pokemon

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Delete a pokemon
# /// @param[in] pokemon_id ID of the pokemon to delete
# /// @param[in] db Database session
# /// @return Success message
# /// @throw HTTPException if pokemon not found
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.delete("/pokemons/{pokemon_id}", response_model=dict)
def delete_pokemon(pokemon_id: int, db: Session = Depends(get_db)):
    pokemon = db.query(Pokemon).filter(Pokemon.id == pokemon_id).first()
    if not pokemon:
        raise HTTPException(status_code=404, detail="Pokemon not found")
    db.delete(pokemon)
    db.commit()
    return {"message": f"Pokemon avec ID {pokemon_id} supprimé."}

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Get a formatted card view of a pokemon
# /// @param[in] pokemon_id ID of the pokemon
# /// @param[in] db Database session
# /// @return Formatted pokemon card data
# /// @throw HTTPException if pokemon not found
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.get("/pokemons/{pokemon_id}/card", response_model=dict)
def get_pokemon_card(pokemon_id: int, db: Session = Depends(get_db)):
    pokemon = db.query(Pokemon).filter(Pokemon.id == pokemon_id).first()
    if not pokemon:
        raise HTTPException(status_code=404, detail="Pokemon not found")
    
    return {
        "title": f"Fiche de {pokemon.name}",
        "content": f"""
        Nom: {pokemon.name}
        Type: {pokemon.type}
        Puissance: {pokemon.power}
        Description: {pokemon.name} est un Pokémon de type {pokemon.type} avec une puissance de {pokemon.power}.
        """
    }

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Initialize the database with default pokemons
# /// @param[in] db Database session
# /// @return Success message and number of pokemons added
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.post("/init-db/", response_model=dict)
def initialize_database(db: Session = Depends(get_db)):
    # Clear existing data
    db.query(Pokemon).delete()
    
    # Add default pokemons
    for pokemon_data in DEFAULT_POKEMONS:
        pokemon = Pokemon(**pokemon_data)
        db.add(pokemon)
    
    db.commit()
    
    return {
        "message": "Base de données initialisée avec succès",
        "pokemons_count": len(DEFAULT_POKEMONS)
    }

# //////////////////////////////////////////////////////////////////////////////////////////////////
# /// @brief Clear the entire database
# /// @param[in] db Database session
# /// @return Success message
# //////////////////////////////////////////////////////////////////////////////////////////////////
@app.post("/clear-db/", response_model=dict)
def clear_database(db: Session = Depends(get_db)):
    # Delete all pokemons
    count = db.query(Pokemon).delete()
    db.commit()
    
    return {
        "message": "Base de données vidée avec succès",
        "pokemons_deleted": count
    }
