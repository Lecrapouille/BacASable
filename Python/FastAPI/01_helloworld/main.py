from fastapi import FastAPI

app = FastAPI()

@app.get("/")
def read_root():
    """
    Root endpoint that returns a welcome message.
    
    Returns:
        dict: A dictionary containing the welcome message
              - message (str): The welcome message
    """
    return {"message": "Bienvenue dans FastAPI!"}

@app.get("/items/{item_id}")
def read_item(item_id: int, q: str = None):
    """
    Endpoint to retrieve details of a specific item.
    
    Args:
        item_id (int): The unique identifier of the item
        q (str, optional): Optional query parameter for filtering
    
    Returns:
        dict: A dictionary containing the item information
              - item_id (int): The item identifier
              - q (str, optional): The provided query parameter
    """
    return {"item_id": item_id, "q": q}
