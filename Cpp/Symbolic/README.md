# Symbolic Manipulations

```
Formula: (x + y)**2
Expanding: 2*x*y + x**2 + y**2
```

"Hello-world" C++ application using the following libraries:
- GINaC.cpp: https://www.ginac.de/
- SymEngine.cpp: https://github.com/symengine/symengine

See the comment of the main function to see how to compile the example.

SymEngine has no documentation, no real examples, do not do derivations.

Links for GINaC:
- https://www-zeuthen.desy.de/acat05/talks/Vollinga.Jens.3/vollinga.pdf
- https://www.ginac.de/tutorial.pdf

# Forth with Symbolic Manipulations

See RPN.cpp file.

Command line example:

```
./prog x y    x x "*" y 2 "*" +    diff x .    4 = x .
```

Where:
- `x y` create two symbols `x` and `y`.
- `x x * y 2 +` create the expression `x^2 + 2 y`.
- `diff x` does the derivation on `x`.
- `.` print the expression.
- `4 = x` affect `4` to `x`.
- `.` show the result which shall be `8`.
