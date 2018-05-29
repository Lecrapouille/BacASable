function t=f(n)
a=rand(n,n);
timer();
y=a*a;
t=timer();
endfunction

function t=g(n)
a=maxplus(rand(n,n));
timer();
y=a*a;
t=timer();
endfunction

// 0.032473 seconds
f(400)
// 0.181024 seconds
g(400)
