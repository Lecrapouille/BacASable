# How to Unit Test C Functions?

Let's imagine you have a C project and you want to unit test a function (named here `example`) calling functions
such as `open` and `read`... How to unit test the function `example`? Once you have understood how to mock C library functions, you will be able to mock any functions.

There are two ways.

## Way 1: Exploit the behavior of weak symbols

Functions `open` and `read` coming from shared libraries are tagged as `weak symbol`, meaning if a `strong symbol` is found in your code, then the `strong symbol` will be used instead of the `weak symbol`, and GCC will not fail by detecting duplicated symbols (which would occur if two (or more) strong functions are defined). What does that mean? Simply that you can implement your own functions `open` and `read`, overriding the "official" `open` and `read` functions.

You can compile the following file (named `weak.c`) with the following command: `gcc -W -Wall weak.c -o prog`
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

In this small project, we will use the idea of weak symbols and use the C++ API Google Tests and Google Mocks
(that you can find at https://github.com/google/googletest) to mock functions `open` and `read` to unit test the C function `example`. Let's suppose that the folder `src/` contains your C project and the folder `tests` contains your C++ unit
tests.

In `src/` you will see C files:
- `example.h`: C header file exporting the C function `example` that we want to test.
- `example.c`: C source file of the function `example` calling functions `open` and `read`.
- `main.c`: This file is not used but shows you how to call the function `example`. You can compile it with the following command: `gcc -W -Wall example.c main.c -o example`.

In `tests/` you will see C++ files:
- `mocks.hpp`: C++ header file exporting mock classes mocking C `open` and `read` functions.
- `mocks.cpp`: C++ source file implementing the mocking C functions `open` and `read`.
- `main.cpp`: Calls Google Tests using mocks to unit test the function `example` from `src/example.h`.

In `mocks.hpp` we define one mock class per C symbol: `ReadMock` for the `read` function,
`OpenMock` for the `open` function... You can even create a single class grouping all functions if preferred.
These mock classes mimic singletons but still use a public constructor. This is a debatable hack to select between
calling the mock method (if constructor is called) or calling the real function (if the singleton instance is set to `nullptr`).

In `mocks.cpp` we define strong C symbols for functions we want to mock (`open` and `read`). Do not forget to use `extern "C"`! In these functions we have an `if-then-else` condition to select between:
- calling the mock method through its singleton
- or calling the real function.

In the case where the mocked method is chosen, you can override it like any virtual method through the Google Mocks API. In the case the real function is desired, since the function symbol is already used by our mock, we have to call the real function symbol thanks to the `dlsym` function.

Now you can mock all your C functions easily and use C++ and Google Tests API to unit test your project :)
See the file `tests/main.cpp` as an example that you can compile with:
```
g++ -W -Wall -Wextra --std=c++11 -I../src mocks.cpp main.cpp -o tests `pkg-config gtest gmock --cflags --libs` -ldl
```

If your project has strong functions, you can either tag them as weak or try to compile these functions as a shared library and exploit the weak behavior.
