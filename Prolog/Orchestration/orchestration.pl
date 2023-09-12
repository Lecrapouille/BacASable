:- include(deck).
:- include(bots).

% List of nodes from list of arcs
node(N) :- arc(N,_); arc(_,N).
nodes(Nodes) :- setof(N, node(N), Nodes).

% http://rlgomes.github.io/work/prolog/2012/05/22/19.00-prolog-and-graphs.html
findapath(X, Y, W, [X,Y], _) :- arc(X, Y, W).
findapath(X, Y, W, [X|P], V) :- \+ member(X, V),
                                 arc(X, Z, W1),
                                 findapath(Z, Y, W2, P, [X|V]),
                                 W is W1 + W2.

:-dynamic(solution/2).
findminpath(X, Y, W, P) :- \+ solution(_, _),
                           findapath(X, Y, W1, P1, []),
                           assertz(solution(W1, P1)),
                           !,
                           findminpath(X, Y, W, P).

findminpath(X, Y, _, _) :- findapath(X, Y, W1, P1, []),
                           solution(W2, P2),
                           W1 < W2,
                           retract(solution(W2, P2)),
                           asserta(solution(W1, P1)),
                           fail.

findminpath(_, _, W, P) :- solution(W,P), retract(solution(W, P)).
