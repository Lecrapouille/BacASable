node(1).
node(2).
node(3).
node(4).
node(5).

% arc(X,Y) :- node(X), node(Y), X \= Y.
arc(1,2).
arc(2,3).
arc(3,4).
%arc(4,1).
arc(4,5).
arc(5,6).

nodevoisins(N,R) :-
    node(N),
    node(R),
    (arc(N,R); arc(R,N)).

chemin(X,Y) :-
    arc(X,Y), !.

chemin(X,Y) :-
    arc(Z,Y),
    arc(X,Z), !.

chemin(X,Y) :-
    arc(X,Z),
    arc(U,Y), chemin(Z,U), !.
