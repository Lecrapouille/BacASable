# Pokédex

Simple database of Pokémons using FastAPI, SQLAlchemy and SQLite, with a web interface for display and interaction.

## FastAPI

FastAPI is a modern, fast (high-performance), web framework for building APIs with Python. It's designed to be easy to use and fast, while taking advantage of Python's type annotations.

This project contains several examples progressing from basic to advanced:
- **01_helloworld**: Basic FastAPI hello world example
- **02_simple_pokemons**: Simple Pokédex without persistence
- **03_sql_pokemons**: Pokédex with SQLAlchemy and SQLite
- **04_sql_html_pokemons**: Full web interface with HTML templates

## Installation

```bash
pip install uvicorn python3-uvicorn python3-fastapi python3-sqlalchemy
```

## Run

```bash
uvicorn main:app --reload
```

Where `main` is the name of the file containing the FastAPI application, and `app` is the name of the FastAPI instance.

Open your browser and navigate to http://127.0.0.1:8000 to test.

## Automatic documentation

FastAPI generates two interactive documentation interfaces automatically:

- Swagger UI : http://127.0.0.1:8000/docs
- ReDoc : http://127.0.0.1:8000/redoc
