# LLM Function Calling

Write your API in your bash profile:

```bash
export GOOGLE_API_KEY="votre clé Google AI Studio"
# ou : export GEMINI_API_KEY="..."
```

Then:

```bash
cd BacASable/Python/FunctionCalling
. .venv/bin/activate
pip install -r requirements.txt
python llm_function_caller.py
```

Clé API : [Google AI Studio](https://aistudio.google.com/apikey) (`GOOGLE_API_KEY` ou `GEMINI_API_KEY`).
