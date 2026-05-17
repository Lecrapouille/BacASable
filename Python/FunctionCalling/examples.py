#!/usr/bin/env python3
"""
Runnable examples for 'llm_function_caller'. From the repo root::

    cd BacASable/Python/FunctionCalling
    . .venv/bin/activate
    export GOOGLE_API_KEY=...
    python examples.py

The first step uses 'choose_job': the model routes to one of two *jobs* (same
'@llm_fill' / function-calling flow) based on what you say in natural language.

List models (after 'generate_content' or 404 on an unknown model id)::

    python examples.py --list-models
    python examples.py --list-models --all
"""

import argparse

from llm_function_caller import ParamMeta, llm_fill, print_available_gemini_models


# -----------------------------------------------------------------------------
# Job: reception kiosk: collect visitor reception details.
#
# Tutorial: this is a typical "form fill" job. The model asks the user for each
# required field in chat, then issues a function_call so Python receives structured
# arguments and runs 'book_visit(...)' once. Optional fields use ParamMeta.required=False.
# -----------------------------------------------------------------------------
@llm_fill(
    system_prompt=(
        "You are front-desk reception. Ask short polite questions in English "
        "to capture the visitor, who they meet, appointment time—then invoke "
        "the registered tool immediately once mandatory fields exist."
    ),
    param_meta={
        "visitor": ParamMeta(
            "Full visitor name",
            examples=["Jean Dupont", "Marie Curie"],
        ),
        "host": ParamMeta(
            "Employee/host they are meeting",
            examples=["Mr Martin", "Dr Bernard"],
        ),
        "time": ParamMeta(
            "Appointment time HH:MM",
            examples=["09:00", "14:30"],
        ),
        "subject": ParamMeta(
            "Reason/subject",
            required=False,
        ),
    },
)
def book_visit(
    visitor: str, host: str, time: str, subject: str = "unspecified"
) -> dict:
    """Register a visitor appointment in the reception system."""
    print("\nRegistered visit:")
    print(f"   Visitor   : {visitor}")
    print(f"   Host      : {host}")
    print(f"   Time      : {time}")
    print(f"   Subject   : {subject}")
    return {"visitor": visitor, "host": host, "time": time, "subject": subject}


# -----------------------------------------------------------------------------
# Job: pizza order: collect delivery details for a fake pizzeria.
#
# Tutorial: 'enum' on ParamMeta constrains the model to allowed pizza names and
# sizes (JSON Schema enums). The job ends when the tool call supplies all required
# fields; Python then prints a confirmation in 'order_pizza'.
# -----------------------------------------------------------------------------
@llm_fill(
    system_prompt=(
        "You assist a fictional pizzeria. Collect pizza order details amiably "
        "(French or English replies from the user are fine). Call the declared "
        "tool once you have everything."
    ),
    param_meta={
        "client": ParamMeta("Customer first name"),
        "pizza": ParamMeta(
            "Ordered pizza variant",
            enum=["Margherita", "Regina", "4 fromages", "Calzone"],
        ),
        "size": ParamMeta("Box size", enum=["S", "M", "L", "XL"]),
        "address": ParamMeta("Full street address"),
    },
)
def order_pizza(client: str, pizza: str, size: str, address: str) -> dict:
    """Finalize a delivery pizza order."""
    print("\nPizza confirmed:")
    print(f"   Client    : {client}")
    print(f"   Pie       : {pizza} ({size})")
    print(f"   Address   : {address}")
    return {"client": client, "pizza": pizza, "size": size, "address": address}


# -----------------------------------------------------------------------------
# Job: router: decide which runnable job to start next.
#
# Tutorial: same decorator pattern, but the tool has a single enum argument 'job'.
# The model maps free-form user intent ("I want the kiosk", "pizza please") to
# 'reception' or 'pizza'. 'main' then calls 'book_visit' or 'order_pizza',
# each of which may run its *own* LLM collection loop—two stacked jobs, two tools.
# -----------------------------------------------------------------------------
@llm_fill(
    system_prompt=(
        "You route the user to one of two interactive jobs. From natural language "
        "(English or French) determine intent: reception / front desk / visitor "
        "appointment ==> job=reception; pizzeria / pizza order ==> job=pizza. "
        "Call the tool as soon as intent is clear; ask at most one short clarifying "
        "question if ambiguous."
    ),
    param_meta={
        "job": ParamMeta(
            "Which job to run next",
            enum=["reception", "pizza"],
            examples=["run the reception kiosk", "I want to order a pizza"],
        ),
    },
)
def choose_job(job: str) -> str:
    """Pick the runnable job via function calling from the user's stated intent."""
    return job

# -----------------------------------------------------------------------------
# Maps tool output 'job' ==> the decorated function that performs that
# job's conversation.
# -----------------------------------------------------------------------------
JOBS = {
    "reception": book_visit,
    "pizza": order_pizza,
}

# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
def main() -> None:
    parser = argparse.ArgumentParser(
        description="Interactive @llm_fill examples (Gemini function calling).",
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

    print(
        "Tell the assistant which job you want: reception kiosk or pizza order.\n"
    )
    selected = choose_job()
    run = JOBS.get(selected, book_visit)
    run()


if __name__ == "__main__":
    main()
