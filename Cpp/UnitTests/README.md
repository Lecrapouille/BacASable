# How to unit tests C functions ?

Let imagine you have a C project and you want to unit test a function (nammed here `example`) calling functions
such as `open` and `read` ... ? How to unit test the function `example`. Once you have understood how to mock Clib functions, you will be able to mock any functions.

There is two ways.

## Way 1: Exploit the behavior of weak symbols

Functions `open` and `read` coming from shared libraries are tagged as `weak symbol` meaning if a `strong symbol` is found in your code then the `strong symbol` will be used instad of `weak symbol` and GCC will not failed by detecting duplicated symbols (which would occured if two (or more) strong functions are defined). What does that means ? Simply that you can implement you own functions `open` and `read` smashing the "official" `open` and `read` functions.

You can compile the following file (named `weak.c`) withe the following command `gcc -W -Wall weak.c -o prog`
```
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

// Comment this function to make call the real open() function
int open(const char *pathname, int flags, ...)
{
   fprintf(stdout, "Hello this is a strong function replacing the real open function\n");
   fprintf(stdout, "Path %s\n", pathname);
   (void) flags;
   return -1;
}

int main()
{
   return open("weak.c", O_RDONLY);
}
```

## Way 2: Mocking with C++ code and Google Tests

In this small project, we will use the idea of weak symbols and use the C++ API Google tests and Google mocks
(that you can find at https://github.com/google/googletest) to mock functions `open` and `read` to unit test the C function `example`. Let suppose that the folder `src/` contains your C project and in the folder `tests` your C++ unit
tests.

In `src/` you will see C files:
- `example.h` C header file exporting the C function `example` that we want to test.
- `example.c` C source file of the fonction `example` calling functions `open` and `read`.
- `main.c` this file is not used but shows you how to call the fonction `example`. You can compile it with the following command `gcc -W -Wall example.c main.c -o example`.

In `tests/` you will see C++ files:
- `mocks.hpp` C++ header file exporting mock class mocking C `open` and `read` functions.
- `mocks.cpp` C++ source file implementing the mocking C function `open` and `read` functions.
- `main.cpp` Call Google tests using mocks to unit tests the function `example` from `src/example.h`.

In `mocks.hpp` we define one mock class by C symbol: `ReadMock` for the `read` function,
`OpenMock` for the `open` function ... You can even create a single class grouping all functions if prefered.
These mock class mimic singleton but still uses a public constructor. This is a debatable hack to select between
calling the mock method (if constructor is called) from calling the real function (if the singleton instance is set to `nullptr`).

In the `mocks.cpp` we define a strong C symbols for functions we want to mock (`open` and `read`). Do not forget to call `extern "C"` ! In these functions we have an `if-then-else` condition to select between:
- calling the mock method through its singleton
- or calling the real function.

In the case where the mocked method is choosed you can override it like any virtual method through the Google mocks API. In the case the real function is desired, since the function symbol is already used by our mock we have to call the next function symbol thanks to the `dlsym` fonction.

Now you can mock all your C fonctions easily and use of C++ and Google Tests API to unit test your project :)
See the file `tests/main.cpp` as example that you can compile with:
```
g++ -W -Wall -Wextra --std=c++11 -I../src mocks.cpp main.cpp -o tests `pkg-config gtest gmock --cflags --libs` -ldl
```

If your project has strong functions, you can either tag them as weak or try to compile these functions as a shared library and to exploit the weak behavior.
