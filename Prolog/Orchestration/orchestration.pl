:- include(deck).
:- include(bots).

% List of nodes from list of arcs
node(N) :- arc(N,_); arc(_,N).
nodes(Nodes) :- setof(N, node(N), Nodes).

% http://rlgomes.github.io/work/prolog/2012/05/22/19.00-prolog-and-graphs.html
findapath(X, Y, [X,Y], _) :- arc(X, Y).
findapath(X, Y, [X|P], V) :- \+ member(X, V), arc(X, Z), findapath(Z, Y, P, [X|V]).
