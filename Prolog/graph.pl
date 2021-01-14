% Based on
% https://rlgomes.github.io/work/prolog/2012/05/22/19.00-prolog-and-graphs.html

:- dynamic node/1.
:- dynamic arc/2.

node(1). % a
node(2). % b
node(3). % c
node(4). % d
node(5). % e
node(6). % f
node(7). % g
node(8). % h

% Directed graph:
%  a --> b ---> e
%  ^           /
%   \         /
%    \       +
%     c <-- d --> f
%            \
%             \
%     h        g

arc(1,2). % a --> b
arc(2,5). % b --> e
arc(3,1). % c --> a
arc(4,3). % d --> c
arc(5,4). % e --> d
arc(4,6). % d --> f
arc(4,7). % d --> g

% Add a cycle: g --> d
arc(7,4).

% arc(X,Y) :- node(X), node(Y), X \= Y.

% Path between node X to node Y
path(X,Y)   :- path(X,Y,[]). % [] is list of visited nodes
path(X,Y,_) :- node(X), node(Y), arc(X,Y).
path(X,Y,V) :-
    \+ member(X,V), % abort if node X is present in the list of visited nodes
    arc(X,Z), path(Z,Y,[X|V]).

% Get nodes reachable from node X. Store them in a set.
reachable(X,List) :- setof(Y, path(X,Y), List).

% Neighbor nodes of node N, stored in R
voisins(N,R) :-
    node(N), node(R),
    (arc(N,R); arc(R,N)).

% Add a node if not already present
add(N) :-
    \+ node(N), assert(node(N)).

% Add an arc if not already present
add(N,M) :-
    \+ arc(N,M),
    (add(N); add(M)),
    assert(arc(N,M)).

% Remove a node
supp(N) :-
    retract(node(N)),
    (retract(arc(_,N)); retract(arc(N,_))).

% Personal implementation for path/2 but also recurse from end
% chemin(X,Y) :- arc(X,Y), !.
% chemin(X,Y) :- arc(Z,Y), arc(X,Z), !.
% chemin(X,Y) :- arc(X,Z), arc(U,Y), chemin(Z,U), !.

% Path of length 0 is a node
chemin(X,X,0) :- node(X).
% Path of length 1 is an arc
chemin(X,Y,1) :- arc(X,Y).
% Path of length > 1
chemin(X,Y,L) :- L > 1, arc(X,Z), M is L - 1, chemin(Z,Y,M).
% Find cycles
cycle(X,L) :- chemin(X,X,L).

% Sparse database
data(node, 1, 43).
data(node, 2, 44).
data(node, 5, 10).
data(node, 6, 11).
data(node, 10, 12).
data(node, 8, 12).
data(arc, 10, 12).

search(N) :- node(N), data(node, N, M), M >= 12.
