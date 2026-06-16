"""
Structured data extraction via LLM (Google Gemini)
====================================================

Uses **function calling** (sometimes called “tool use”) as the mechanism: you
declare one tool whose shape mirrors a Python function. The model either asks
the user follow-up questions in natural language or emits a structured payload
with argument values extracted from the conversation.

Tutorial mental model:

1. You write a plain Python function.
2. '@llm_fill' attaches a wrapper that introspects 'inspect.signature' plus type hints.
3. The wrapper derives a Gemini 'FunctionDeclaration' (name, description, JSON-like parameter schema).
4. In a terminal loop we send messages to Gemini; when the model issues a tool call with the right name, we read 'args' and invoke your underlying function.

Python Requirements: pip install -r requirements.txt  
Authentication: GOOGLE_API_KEY or GEMINI_API_KEY
See Google AI Studio: https://aistudio.google.com/apikey.
"""

from __future__ import annotations

import inspect, json, os
from dataclasses import dataclass, field
from typing import Any, Callable, Iterator, Optional, get_type_hints

from google import genai
from google.genai import types


# -----------------------------------------------------------------------------
# Extra metadata per parameter (feeds richer tool schema for the model)
# -----------------------------------------------------------------------------

@dataclass
class ParamMeta:
    """How to describe one parameter inside the Gemini tool declaration."""

    # Shown inside the Schema “description”: guides both the UI and the model.
    # Example:
    #   description: "Full visitor name"
    description: str

    # If False, omit from Gemini “required” list (caller may still omit if default).
    required: bool = True

    # Concatenated into description as examples—the model learns valid shapes.
    # Example:
    #   examples: ["Jean Dupont", "Marie Curie"]
    examples: list[str] = field(default_factory=list)

    # Optional JSON Schema enum: restricts allowed string values when provided.
    # Example:
    #   enum: ["pizza", "pasta", "salad"]
    enum: Optional[list[str]] = None

# -----------------------------------------------------------------------------
# The decorator (@llm_fill)
# -----------------------------------------------------------------------------
#
# Pattern: decorator factory: 'llm_fill(...)' returns the real decorator that
# receives your function definition.
#
#   @llm_fill(...)            # <-- step A: configure (system prompt, model, metadata)
#   def my_tool(x: int): ...  # <-- step B: decorator receives 'my_tool'
#
# Behind the scenes we replace 'my_tool' with 'wrapper': same name but when
# you call 'my_tool()' without filling every positional arg, 'wrapper' runs the
# LLM-driven collection loop and then forwards 'func(**collected_args)'.
# -----------------------------------------------------------------------------

def llm_fill(
    system_prompt: str = (
        "You are a friendly assistant. Ask short polite questions in English "
        "to collect any missing fields."
    ),
    model: str = "models/gemini-3.1-flash-lite",
    param_meta: Optional[dict[str, ParamMeta]] = None,
):
    """Turn 'func(...):' into an interactive Gemini-powered collector + call.

    The factory returns 'decorator(func)'. That inner decorator binds:
      - the untouched original 'func'.
      - a precomputed tool declaration derived from signatures + hints.
      - the 'wrapper' exposed as the callable you import/call later.
    """

    def decorator(func: Callable) -> Callable:
        # Snapshot the Python callable so we never guess arity at runtime blindly.
        sig = inspect.signature(func)
        hints = get_type_hints(func)

        # One dict per decorated function; reused across many 'wrapper' calls.
        tool_decl = _build_tool_declaration(func, sig, hints, param_meta or {})

        def wrapper(*args, **kwargs):
            # Fast path: programmer already satisfied every positional parameter.
            # (`set(kwargs)==set(...)' catches the “passed every keyword once” route.
            # When that happens there is zero reason to pay for an HTTP round-trip.
            if len(args) == len(sig.parameters) or set(kwargs) == set(sig.parameters):
                return func(*args, **kwargs)

            # Slow path: run the Gemini chat-style loop until JSON args appear.
            collected = _run_interactive_collection(
                func=func,
                tool_decl=tool_decl,
                system_prompt=system_prompt,
                model=model,
            )
            # 'collected' should match keywords accepted by 'func'.
            return func(**collected)

        # Debugging hooks + preserve docstrings for 'help()' users.
        wrapper.__wrapped__ = func
        wrapper.__doc__ = func.__doc__
        return wrapper

    return decorator


# -----------------------------------------------------------------------------
# Map Python annotations ==> Gemini JSON-ish types ==> 'types.Schema' enums
#
# Gemini expects 'types.Type.STRING' ... not raw strings—the dict below
# resolves the hop from textual JSON Schema names to Google's enum literals.
# -----------------------------------------------------------------------------

_PYTHON_TO_JSON_TYPE = {
    str: "string",
    int: "integer",
    float: "number",
    bool: "boolean",
    list: "array",
    dict: "object",
}

_JSON_STR_TO_GENAI_TYPE = {
    "string": types.Type.STRING,
    "integer": types.Type.INTEGER,
    "number": types.Type.NUMBER,
    "boolean": types.Type.BOOLEAN,
    "array": types.Type.ARRAY,
    "object": types.Type.OBJECT,
}


def _build_tool_declaration(
    func: Callable,
    sig: inspect.Signature,
    hints: dict,
    param_meta: dict[str, ParamMeta],
) -> dict[str, Any]:
    """Build a neutral `{name, description, properties, required}` declaration.

    Gemini's Python SDK consumes 'types.Schema' objects; splitting this step lets
    the rest of the file stay provider-agnostic if you ever bolt on another vendor.
    """
    properties: dict[str, Any] = {}
    required: list[str] = []

    for name, param in sig.parameters.items():
        # Default to 'str' if the author omitted type hints altogether.
        py_type = hints.get(name, str)
        json_type = _PYTHON_TO_JSON_TYPE.get(py_type, "string")

        meta = param_meta.get(name, ParamMeta(f"Value for parameter `{name}`"))

        prop: dict[str, Any] = {
            "type": json_type,
            "description": meta.description,
        }
        if meta.examples:
            prop["description"] += f" (e.g. {', '.join(meta.examples)})"
        if meta.enum:
            prop["enum"] = meta.enum

        properties[name] = prop

        # Gemini “required” array lists parameters that MUST be synthesized by the LLM.
        has_default = param.default is not inspect.Parameter.empty
        if meta.required and not has_default:
            required.append(name)

    return {
        "name": func.__name__,
        "description": (
            inspect.cleandoc(func.__doc__ or "")
            or f"Call `{func.__name__}` with the correct keyword arguments."
        ),
        "properties": properties,
        "required": required,
    }


def _declaration_to_gemini_tool(decl: dict[str, Any]) -> types.Tool:
    """Materialize Gemini 'Tool' wrappers from the neutral declaration dict."""
    schema_props: dict[str, types.Schema] = {}
    for pname, p in decl["properties"].items():
        jt = p["type"]
        genai_type = _JSON_STR_TO_GENAI_TYPE.get(jt, types.Type.STRING)
        kwargs: dict[str, Any] = {
            "type": genai_type,
            "description": p.get("description"),
        }
        if "enum" in p:
            kwargs["enum"] = p["enum"]
        schema_props[pname] = types.Schema(**kwargs)

    parameters = types.Schema(
        type=types.Type.OBJECT,
        properties=schema_props,
        required=decl.get("required") or None,
    )

    fn_decl = types.FunctionDeclaration(
        name=decl["name"],
        description=decl["description"],
        parameters=parameters,
    )
    return types.Tool(function_declarations=[fn_decl])


def _gemini_api_key() -> str:
    key = os.environ.get("GOOGLE_API_KEY") or os.environ.get("GEMINI_API_KEY")
    if not key:
        raise RuntimeError(
            "Missing API key. Set GOOGLE_API_KEY or GEMINI_API_KEY "
            "(create one at https://aistudio.google.com/apikey )."
        )
    return key


# -----------------------------------------------------------------------------
# Discover remote model IDs ('ModelService.ListModels')
# -----------------------------------------------------------------------------
#
# A '404 ... model ... is not found' from 'generate_content' almost always means
# the string passed to 'model=' is not published for *this* API key / endpoint.
# The listing below comes straight from Google: each row shows the **short id**
# you should pass to '@llm_fill(model=...)' (the segment after the final
# '/models/' in the resource name). 'supported_actions' mirrors the API’s
# 'supportedGenerationMethods' field—your decorator needs 'generateContent'.
# -----------------------------------------------------------------------------

_GEMINI_GENERATE_CONTENT = "generateContent"


def model_api_id(model: types.Model) -> str:
    """Gemini HTTP API wants the suffix after '.../models/' (not the full resource path)."""
    raw = (model.name or "").strip()
    if "/models/" in raw:
        return raw.split("/models/", 1)[-1]
    return raw


def iter_gemini_models(
    *,
    only_generate_content: bool = True,
    client: Optional[genai.Client] = None,
) -> Iterator[types.Model]:
    """Yield models returned by 'client.models.list()' (paginates automatically).

    When 'only_generate_content' is True, skip entries whose
    'supported_actions' list does not advertise 'generateContent'—those IDs
    would fail inside our chat loop even though they might exist for other RPCs.
    """
    c = client or genai.Client(api_key=_gemini_api_key())
    for m in c.models.list():
        if only_generate_content:
            acts = m.supported_actions or []
            if _GEMINI_GENERATE_CONTENT not in acts:
                continue
        yield m


def print_available_gemini_models(*, only_generate_content: bool = True) -> None:
    """Print a human-readable table to stdout (queries the live Gemini API)."""
    models = sorted(
        iter_gemini_models(only_generate_content=only_generate_content),
        key=lambda m: model_api_id(m).lower(),
    )

    if only_generate_content:
        print(
            "Models with generateContent (pass the short id to @llm_fill(model=...)):\n"
        )
    else:
        print("All models visible to this API key:\n")

    if not models:
        print("  (empty: check API key or try --list-models with --all)\n")
        return

    for m in models:
        mid = model_api_id(m)
        methods = ", ".join(m.supported_actions or []) or "(no methods listed)"
        print(f"  - {mid}")
        if m.display_name and m.display_name != mid:
            print(f"      display_name : {m.display_name}")
        print(f"      methods      : {methods}")
        print(f"      resource     : {m.name}")
        print()


def _model_text_response(content: Optional[types.Content]) -> str:
    """Collect every plaintext 'Part' Gemini returned inside one turn."""
    if not content or not content.parts:
        return ""
    lines: list[str] = []
    for part in content.parts:
        if part.text:
            lines.append(part.text)
    return "\n".join(lines).strip()


def _find_function_call_args(
    content: Optional[types.Content], func_name: str
) -> Optional[dict[str, Any]]:
    """Locate the structured args dict that matches 'func.__name__'."""
    if not content or not content.parts:
        return None
    for part in content.parts:
        fc = part.function_call
        if fc and fc.name == func_name and fc.args:
            return dict(fc.args)
    return None


# -----------------------------------------------------------------------------
# Interactive human ↔ Gemini loop (function calling)
#
# Gemini returns either:
#   - conversational 'Part.text' (ask user for clarification), or
#   - 'Part.function_call' carrying JSON-like 'args'.
#
# We disable **automatic_function_calling** so the SDK does not execute Python on
# Google's side—instead we faithfully mirror “LLM emits JSON ==> we call 'func'.
# -----------------------------------------------------------------------------

def _run_interactive_collection(
    func: Callable,
    tool_decl: dict[str, Any],
    system_prompt: str,
    model: str,
) -> dict[str, Any]:
    """Send/receive Gemini messages until 'function_call.args' pops out."""
    client = genai.Client(api_key=_gemini_api_key())
    gemini_tool = _declaration_to_gemini_tool(tool_decl)

    print(f"\n[LLM] Starting collection for `{func.__name__}` (Gemini)...\n")

    # 'contents' is the entire chat transcript in Gemini-native 'Content' form.
    contents: list[types.Content] = [
        types.Content(
            role="user",
            parts=[
                types.Part.from_text(
                    text=(
                        f"I will invoke `{func.__name__}`. "
                        "Ask me whatever clarifications you need in natural language, "
                        "then invoke the declared tool exactly once everything is filled."
                    )
                ),
            ],
        ),
    ]

    gen_cfg = types.GenerateContentConfig(
        system_instruction=system_prompt,
        tools=[gemini_tool],
        tool_config=types.ToolConfig(
            function_calling_config=types.FunctionCallingConfig(
                # AUTO lets the assistant decide freely between chatter & tool emits.
                mode=types.FunctionCallingConfigMode.AUTO,
            ),
        ),
        automatic_function_calling=types.AutomaticFunctionCallingConfig(disable=True),
    )

    while True:
        response = client.models.generate_content(
            model=model,
            contents=contents,
            config=gen_cfg,
        )

        # Safety/policy refusals can yield empty 'candidates'—surface loudly.
        if not response.candidates:
            pf = response.prompt_feedback
            raise RuntimeError(f"No candidates returned by Gemini (prompt_feedback={pf!r}).")

        model_content = response.candidates[0].content
        contents.append(model_content)

        args = _find_function_call_args(model_content, func.__name__)
        if args is not None:
            print(
                f"\n[LLM] Tool call resolved to:\n"
                f"{json.dumps(args, ensure_ascii=False, indent=2)}\n"
            )
            return args

        reply = _model_text_response(model_content)
        if reply:
            print(f"[Assistant] {reply}")

        user_input = input("[You] ").strip()
        if not user_input:
            continue

        contents.append(
            types.Content(
                role="user",
                parts=[types.Part.from_text(text=user_input)],
            ),
        )
