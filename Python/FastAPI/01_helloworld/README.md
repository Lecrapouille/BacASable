```bash
uvicorn main:app --reload
```

- Type the following URL in your browser: `http://127.0.0.1:8000/` will return `{"message": "Bienvenue dans FastAPI!"}`
- Type the following URL in your browser: `http://127.0.0.1:8000/items/42?q=hello` will return `{"item_id": 42, "q": "hello"}`