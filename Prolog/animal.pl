% From https://www.metalevel.at/prolog/expertsystems

is_true(Q) :-
    format("~w?\n", [Q]),
    read(yes).

% Naive way: questions may be repeated (Has fur? no. Has fur? no. ...)
animal(dog)  :- is_true('has fur'), is_true('says woof').
animal(cat)  :- is_true('has fur'), is_true('says meow').
animal(duck) :- is_true('has feathers'), is_true('says quack').

%
animals([animal(dog, [is_true('has fur'), is_true('says woof')]),
         animal(cat, [is_true('has fur'), is_true('says meow')]),
         animal(duck, [is_true('has feathers'), is_true('says quack')])]).

animal2(A) :-
    animals(As),
    member(animal(A,Cs), As),
    maplist(call, Cs).
