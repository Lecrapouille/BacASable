# Call C function from Julia

## What ?

In this example `foo.c` contains two C functions are using a structure holding a pointer to an external array of data (of type double) and the size of this array. The first function `updatestruct` sets the states (pointer and size) and the second function `recvstruct` displays the content.

The `foo.jl` contains the two Julia functions wrapping the two C functions.

## How ?

- Step 1: Compile the c file to a shared library.

```sh
gcc foo.c -fpic -shared -o libfoo.so
```

- Step 2: Run the Julia code.

```sh
julia foo.jl
```

- Result:

```
mystruct=1, 2, 3 (3)
[1.0, 2.0, 3.0]

```
