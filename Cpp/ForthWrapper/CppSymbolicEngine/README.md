# Forth with Symbolic Manipulations

Implementation in C++ of an ultra-basic Forth interpreter wrapping a thin C++ symbolic manipulation library Ginac https://www.ginac.de/ based on https://github.com/udexon/SymForth. See ../Symbolic folder for a classic Ginac hello world application.

You can find the youtube video of this code at https://youtu.be/e0z6qGpBc8I
And the PowerPoint file used for making this video: https://lecrapouille.github.io/forth/ForthWrapper_CppSymbolicEngine.odp

Compilation:

```
g++ --std=c++14 -W -Wall RPN.cpp -o prog `pkg-config ginac --libs --cflags`
```

Command line example:

```
./prog x y    x x "*" y 2 "*" +    diff x .    4 = x .
```

Where:
- `x y` creates two symbols `x` and `y`.
- `x x * y 2 +` creates the expression `x^2 + 2 y`.
- `diff x` does the derivation on `x`.
- `.` prints the expression.
- `4 = x` assigns `4` to `x`.
- `.` shows the result which shall be `8`.
- *Note:* you have to type `"*"` instead of `*`
to prevent bash from expanding it with names of the whole files present in the folder.
