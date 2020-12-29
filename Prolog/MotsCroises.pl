mot(astante, a,s,t,a,n,t,e).
mot(astoria, a,s,t,o,r,i,a).
mot(baratto, b,a,r,a,t,t,o).
mot(cobalto, c,o,b,a,l,t,o).
mot(pistola, p,i,s,t,o,l,a).
mot(statale, s,t,a,t,a,l,e).

motscroises(V1,V2,V3,H1,H2,H3) :-
    mot(V1,_,A,_,B,_,C,_),
    mot(V2,_,D,_,E,_,F,_),
    mot(V3,_,G,_,H,_,I,_),
    mot(H1,_,A,_,D,_,G,_),
    mot(H2,_,B,_,E,_,H,_),
    mot(H3,_,C,_,F,_,I,_).
