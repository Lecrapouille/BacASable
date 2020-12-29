# Prolog and Graph

Install SWI Prolog:
- `https://github.com/SWI-Prolog/swipl-devel`

Get fun with the prolog graph:
- From Linux console, type: `swipl -s graph.pl`
- Fron SWI Prolog prompt, type: `path(1,X).` to get reachable nodes from node 1. Type `;` to find others.
- Fron SWI Prolog prompt, type: `reachable(1,L).` to get reachable nodes from node 1 stored in a set (no duplicated nodes).
- Fron SWI Prolog prompt, type: `chemin(X,X,5).` to find a cycle path of length 5.
- To trace: `trace.`.
- Note: once in the prompt, you cannot add new clauses such `node(6).` since you are interrogating the database.
 To add: `assert(node(6)).` To remove: `retract(node(1)).` Look at clauses: `listing.`
- To add or remove the node 10: `add(10)` or `supp(10)`.

C++ calling SWI Prolog:
- ``g++ -W -Wall graph.cpp `pkg-config swipl --cflags --libs` -o prog``
