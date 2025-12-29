# AutoLoadLib

Interactive Programming in C: modify your C code at runtime, see your changes in your application without restarting it.
Simulate your C program as an interactive language.

Inspired by:
* https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus
* https://github.com/mazbox/livecode-cplusplus
* http://nullprogram.com/blog/2014/12/23/

## Tutorial steps
* Step01: Load and run a function from a shared library.
* Step02: Auto-compile-and-load a function from a shared library. Modify the lib.c file and
when saving it, see the impact on the console.

## Prerequisites

Install: g++, gtkmm3. You need to install gtkmm3 for compiling the cpp file.
Indeed gtkmm3 has an architecture-independent dynamic library loader. This will
simplify the source code for opening a dynamic library without writing different code
depending on Windows, Linux, or OSX.

Ubuntu:
```
sudo apt-get install libgtkmm-3.0-dev
```

OS X:
```
brew install gtkmm3
```

## Compilation

```
make launch
```

Will compile and launch the example. For Step02, gedit will be launched for letting you
modifying the code of lib.c.

Note: For compiling the shared library for operating systems other than Linux and OSX, you may have to
adapt the Makefile script. Feel free to report an issue.

## Future improvements

* Currently we load only C code. No C++ code yet!
* No segfault protection.
* Will be used for my [Forth C loader](https://github.com/Lecrapouille/SimTaDyn).