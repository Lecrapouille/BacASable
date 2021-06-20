# Symbolic Manipulation With C++/Forth

Symbolic Manipulation, also known as `computer algebra` or `algebraic computation`,
allow to manipulate mathematical expressions and mathematical objects. For example,
expanding the following formula `(x + y)**2` would give `2*x*y + x**2 + y**2`.

C++ and Forth are languages that does not offer builtin computer algebra. We need
some libraries :
- GINaC.cpp: https://www.ginac.de/
- SymEngine.cpp: https://github.com/symengine/symengine

In this folder, I tested some C++ "Hello-world" algebraic computation with these
two libraries: GINaC.cpp and SymEngine.cpp.

Here are my personal feeling concerning these libraries:
- SymEngine has no documentation, no real examples, do not do derivations. Making
  a basic example was a pain.
- GINaC was really more faster to create an build my first application. Links for GINaC:
  - https://www-zeuthen.desy.de/acat05/talks/Vollinga.Jens.3/vollinga.pdf
  - https://www.ginac.de/tutorial.pdf

Finally, I tested how easy these libraries can be used by a token Forth made in C++.
See the comment of the main function to see how to compile examples: RPN.cpp (note:
RPN means Reverse Polish Notation and the Forth langage uses RPN). Note: the same file
is also present in ../ForthWrapper/CppSymbolicEngine
