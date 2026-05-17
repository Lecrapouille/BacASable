#!/usr/bin/env python3
"""
Runnable demos for ``llm_function_caller``. Run from repo root::

    cd BacASable/Python/FunctionCalling
    . .venv/bin/activate
    export GOOGLE_API_KEY=...
    python examples.py

List models (after ``generate_content`` / 404 on unknown model id)::

    python examples.py --list-models
    python examples.py --list-models --all
"""

import argparse

from llm_function_caller import ParamMeta, llm_fill, print_available_gemini_models


@llm_fill(
    system_prompt=(
        "You are front-desk reception. Ask short polite questions in English "
        "to capture the visitor, who they meet, appointment time—then invoke "
        "the registered tool immediately once mandatory fields exist."
    ),
    param_meta={
        "visiteur": ParamMeta(
            "Full visitor name",
            examples=["Jean Dupont", "Marie Curie"],
        ),
        "hote": ParamMeta(
            "Employee/host they are meeting",
            examples=["Mr Martin", "Dr Bernard"],
        ),
        "heure": ParamMeta(
            "Appointment time HH:MM",
            examples=["09:00", "14:30"],
        ),
        "objet": ParamMeta(
            "Reason/subject",
            required=False,
        ),
    },
)
def book_visit(
    visiteur: str, hote: str, heure: str, objet: str = "unspecified"
) -> dict:
    """Register visitor appointment in reception system."""
    print("\nRegistered visit:")
    print(f"   Visitor   : {visiteur}")
    print(f"   Host      : {hote}")
    print(f"   Time      : {heure}")
    print(f"   Subject   : {objet}")
    return {"visiteur": visiteur, "hote": hote, "heure": heure, "objet": objet}


@llm_fill(
    system_prompt=(
        "You assist a fictional pizzeria. Collect pizza order details amiably "
        "(French or English replies are fine). Call the declared tool once you "
        "have everything."
    ),
    param_meta={
        "client": ParamMeta("Customer first name"),
        "pizza": ParamMeta(
            "Ordered pizza variant",
            enum=["Margherita", "Regina", "4 fromages", "Calzone"],
        ),
        "taille": ParamMeta("Box size", enum=["S", "M", "L", "XL"]),
        "adresse": ParamMeta("Full street address"),
    },
)
def order_pizza(client: str, pizza: str, taille: str, adresse: str) -> dict:
    """Finalize delivery pizza order."""
    print("\nPizza confirmed:")
    print(f"   Client    : {client}")
    print(f"   Pie       : {pizza} ({taille})")
    print(f"   Address   : {adresse}")
    return {"client": client, "pizza": pizza, "taille": taille, "adresse": adresse}


EXAMPLES = {
    "1": book_visit,
    "2": order_pizza,
}


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Interactive @llm_fill demos (Gemini function calling).",
    )
    parser.add_argument(
        "--list-models",
        action="store_true",
        help=(
            "Query the Gemini API for model IDs that support generateContent. "
            "Use a printed short id in @llm_fill(model=...) if you get 404 NOT_FOUND."
        ),
    )
    parser.add_argument(
        "--all",
        action="store_true",
        dest="list_all",
        help="With --list-models, include models that do not advertise generateContent.",
    )
    args = parser.parse_args()

    if args.list_models:
        print_available_gemini_models(only_generate_content=not args.list_all)
        return

    print("Choose a demo:")
    print("  1 – Reception kiosk")
    print("  2 – Pizza order")
    choice = input("Pick 1 / 2: ").strip()

    EXAMPLES.get(choice, book_visit)()


if __name__ == "__main__":
    main()
