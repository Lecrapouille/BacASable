# Prolog and Graph

Learning programming Prolog. Learning how to call Prolog from C++ code. Implementing ultra basic Prolog routines based
on graph theory that could be used inside my personal project SimTaDyn instead of heavy C++ class and code. This is a
POC code.

This code has be tested on SWI Prolog. To install it:
- `https://github.com/SWI-Prolog/swipl-devel`

To get fun with the prolog graph:
- From Linux console, type: `swipl -s graph.pl`
- Nodes are in the way of `node(1).` and directed arcs `arc(1,2).`
- To add or remove the node 10: `add(10)` or `supp(10)`. What, as beginer, did not, once loaded your Prolog files, the database of knowledges is read only. Once in the prompt, you cannot add new clauses such `node(6).` since you are interrogating the database. To add: `assert(node(6)).` To remove: `retract(node(1)).` To look at clauses type `listing.`
- Fron SWI Prolog prompt, type: `path(1,X).` to get reachable nodes from node 1. Type `;` to find others.
- Fron SWI Prolog prompt, type: `reachable(1,L).` to get reachable nodes from node 1 stored in a set (no duplicated nodes).
- Fron SWI Prolog prompt, type: `chemin(X,X,5).` to find a cycle path of length 5.
- To trace for debuggin a clause `trace.`.

C++ calling SWI Prolog:
- ``g++ -W -Wall --std=c++11 graph.cpp `pkg-config swipl --cflags --libs` -o prog``

Note on Mac OS X with direct installation through homebrew, the pkg-config file is missing witch is problematic for using Prolog from C++ and I did not wanted to use `swipl-ld`. As consequence I compiled it from GitHub.
