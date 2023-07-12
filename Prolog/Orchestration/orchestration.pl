:- include(deck).
:- include(bots).

noeud(N) :- arc(N,_); arc(_,N).
noeuds(Nodes) :- setof(N, noeud(N), Nodes).
