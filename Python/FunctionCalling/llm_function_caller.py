"""
Generic LLM Function Caller
============================
Utilise le function calling de Google Gemini pour collecter interactivement
les arguments d'une fonction Python quelconque.

Le LLM pose des questions en langage naturel, extrait les réponses de
l'utilisateur, et appelle la fonction cible une fois tous les paramètres
renseignés.

Dépendances : pip install -r requirements.txt
Variable d'environnement : GOOGLE_API_KEY ou GEMINI_API_KEY (Google AI Studio).
"""

import inspect
import json
import os
from dataclasses import dataclass, field
from typing import Any, Callable, Optional, get_type_hints

from google import genai
from google.genai import types


# ---------------------------------------------------------------------------
# Métadonnées enrichies pour chaque paramètre
# ---------------------------------------------------------------------------

@dataclass
class ParamMeta:
    """Description enrichie d'un paramètre pour le LLM."""
    description: str                        # Question / description pour le LLM
    required: bool = True
    examples: list[str] = field(default_factory=list)
    enum: Optional[list[str]] = None        # Valeurs autorisées


# ---------------------------------------------------------------------------
# Décorateur principal
# ---------------------------------------------------------------------------

def llm_fill(
    system_prompt: str = "Tu es un assistant accueil. Pose des questions courtes et polies pour collecter les informations manquantes.",
    model: str = "gemini-2.5-flash",
    param_meta: Optional[dict[str, ParamMeta]] = None,
):
    """
    Décorateur qui transforme une fonction ordinaire en une fonction dont
    les arguments sont collectés interactivement par un LLM.

    Usage
    -----
    @llm_fill(
        system_prompt="Tu es un agent d'accueil...",
        param_meta={
            "visiteur": ParamMeta("Nom complet du visiteur"),
            "heure":    ParamMeta("Heure du rendez-vous (HH:MM)", examples=["09:00", "14:30"]),
            "hote":     ParamMeta("Nom de la personne à rencontrer"),
        }
    )
    def rendez_vous(visiteur: str, heure: str, hote: str) -> dict:
        return {"visiteur": visiteur, "heure": heure, "hote": hote}
    """
    def decorator(func: Callable) -> Callable:
        # --- Inspecter la signature de la fonction cible ---
        sig = inspect.signature(func)
        hints = get_type_hints(func)

        tool_decl = _build_tool_declaration(func, sig, hints, param_meta or {})

        def wrapper(*args, **kwargs):
            # Si tous les args sont déjà fournis, appel direct
            if len(args) == len(sig.parameters) or set(kwargs) == set(sig.parameters):
                return func(*args, **kwargs)

            # Sinon : laisser le LLM collecter les arguments
            collected = _run_interactive_collection(
                func=func,
                tool_decl=tool_decl,
                system_prompt=system_prompt,
                model=model,
            )
            return func(**collected)

        wrapper.__wrapped__ = func
        wrapper.__doc__ = func.__doc__
        return wrapper

    return decorator


# ---------------------------------------------------------------------------
# Construction du schéma (description de fonction pour Gemini)
# ---------------------------------------------------------------------------

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
    """Construit une déclaration générique (JSON-schema-like) pour le tool."""
    properties: dict[str, Any] = {}
    required: list[str] = []

    for name, param in sig.parameters.items():
        py_type = hints.get(name, str)
        json_type = _PYTHON_TO_JSON_TYPE.get(py_type, "string")

        meta: ParamMeta = param_meta.get(name, ParamMeta(f"Valeur de '{name}'"))

        prop: dict[str, Any] = {
            "type": json_type,
            "description": meta.description,
        }
        if meta.examples:
            prop["description"] += f" (ex : {', '.join(meta.examples)})"
        if meta.enum:
            prop["enum"] = meta.enum

        properties[name] = prop

        has_default = param.default is not inspect.Parameter.empty
        if meta.required and not has_default:
            required.append(name)

    return {
        "name": func.__name__,
        "description": (
            func.__doc__ or f"Appelle la fonction `{func.__name__}` avec les bons arguments."
        ),
        "properties": properties,
        "required": required,
    }


def _declaration_to_gemini_tool(decl: dict[str, Any]) -> types.Tool:
    """Convertit la déclaration en types.Tool Gemini."""
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
            "Clé API manquante. Définissez GOOGLE_API_KEY ou GEMINI_API_KEY "
            "(clé créée sur https://aistudio.google.com/apikey )."
        )
    return key


def _model_text_response(content: Optional[types.Content]) -> str:
    if not content or not content.parts:
        return ""
    lines: list[str] = []
    for part in content.parts:
        if part.text:
            lines.append(part.text)
    return "\n".join(lines).strip()


def _find_function_call_args(content: Optional[types.Content], func_name: str) -> Optional[dict[str, Any]]:
    if not content or not content.parts:
        return None
    for part in content.parts:
        fc = part.function_call
        if not fc:
            continue
        if fc.name == func_name and fc.args:
            return dict(fc.args)
    return None


# ---------------------------------------------------------------------------
# Boucle de collecte interactive (Gemini)
# ---------------------------------------------------------------------------

def _run_interactive_collection(
    func: Callable,
    tool_decl: dict[str, Any],
    system_prompt: str,
    model: str,
) -> dict[str, Any]:
    """
    Boucle humain ↔ LLM jusqu'à ce que le LLM décide d'appeler le tool.
    Retourne le dict d'arguments collectés.
    """
    client = genai.Client(api_key=_gemini_api_key())
    gemini_tool = _declaration_to_gemini_tool(tool_decl)

    print(f"\n[LLM] Démarrage de la collecte pour `{func.__name__}` (Gemini)...\n")

    contents: list[types.Content] = [
        types.Content(
            role="user",
            parts=[
                types.Part.from_text(
                    text=(
                        f"Je vais appeler la fonction `{func.__name__}`. "
                        "Pose-moi les questions nécessaires pour collecter les informations, "
                        "puis appelle le tool quand tu as tout ce qu'il faut."
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
                mode=types.FunctionCallingConfigMode.AUTO,
            ),
        ),
        # Nous interprétons nous-mêmes l'appel de fonction (args → fonction Python).
        automatic_function_calling=types.AutomaticFunctionCallingConfig(disable=True),
    )

    while True:
        response = client.models.generate_content(
            model=model,
            contents=contents,
            config=gen_cfg,
        )

        if not response.candidates:
            pf = response.prompt_feedback
            raise RuntimeError(f"Aucune réponse du modèle (prompt_feedback={pf!r}).")

        model_content = response.candidates[0].content
        contents.append(model_content)

        args = _find_function_call_args(model_content, func.__name__)
        if args is not None:
            print(
                f"\n[LLM] → Appel du tool avec : "
                f"{json.dumps(args, ensure_ascii=False, indent=2)}\n"
            )
            return args

        reply = _model_text_response(model_content)
        if reply:
            print(f"[Assistant] {reply}")

        user_input = input("[Vous] ").strip()
        if not user_input:
            continue

        contents.append(
            types.Content(
                role="user",
                parts=[types.Part.from_text(text=user_input)],
            ),
        )


# ---------------------------------------------------------------------------
# Exemple d'utilisation : gestion de rendez-vous
# ---------------------------------------------------------------------------

@llm_fill(
    system_prompt=(
        "Tu es l'agent d'accueil d'une entreprise. "
        "Pose des questions courtes et polies en français pour identifier le visiteur, "
        "la personne qu'il vient voir et l'heure du rendez-vous. "
        "Dès que tu as les trois informations, appelle le tool immédiatement."
    ),
    param_meta={
        "visiteur": ParamMeta(
            "Nom complet du visiteur",
            examples=["Jean Dupont", "Marie Curie"],
        ),
        "hote": ParamMeta(
            "Nom de l'employé ou du responsable à rencontrer",
            examples=["M. Martin", "Dr. Bernard"],
        ),
        "heure": ParamMeta(
            "Heure du rendez-vous au format HH:MM",
            examples=["09:00", "14:30"],
        ),
        "objet": ParamMeta(
            "Objet ou motif du rendez-vous",
            required=False,
        ),
    },
)
def rendez_vous(visiteur: str, hote: str, heure: str, objet: str = "Non précisé") -> dict:
    """Enregistre un rendez-vous visiteur dans le système d'accueil."""
    print("\n✅ Rendez-vous enregistré :")
    print(f"   Visiteur : {visiteur}")
    print(f"   Reçu par : {hote}")
    print(f"   Heure    : {heure}")
    print(f"   Objet    : {objet}")
    return {"visiteur": visiteur, "hote": hote, "heure": heure, "objet": objet}


# ---------------------------------------------------------------------------
# Exemple générique supplémentaire : commande de pizza
# ---------------------------------------------------------------------------

@llm_fill(
    system_prompt=(
        "Tu es un assistant pour une pizzeria. Collecte les informations de commande "
        "de façon conviviale, puis appelle le tool dès que tu as tout."
    ),
    param_meta={
        "client":   ParamMeta("Prénom du client"),
        "pizza":    ParamMeta("Type de pizza", enum=["Margherita", "Regina", "4 fromages", "Calzone"]),
        "taille":   ParamMeta("Taille", enum=["S", "M", "L", "XL"]),
        "adresse":  ParamMeta("Adresse de livraison complète"),
    },
)
def commander_pizza(client: str, pizza: str, taille: str, adresse: str) -> dict:
    """Passe une commande de pizza avec livraison."""
    print("\n🍕 Commande validée :")
    print(f"   Client  : {client}")
    print(f"   Pizza   : {pizza} ({taille})")
    print(f"   Livraison : {adresse}")
    return {"client": client, "pizza": pizza, "taille": taille, "adresse": adresse}


# ---------------------------------------------------------------------------
# Point d'entrée
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    examples = {"1": rendez_vous, "2": commander_pizza}
    print("Choisissez un exemple :")
    print("  1 - Gestion de rendez-vous")
    print("  2 - Commande de pizza")
    choice = input("Votre choix : ").strip()

    fn = examples.get(choice, rendez_vous)
    fn()  # Le décorateur prend la main
