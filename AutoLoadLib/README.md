# AutoLoadLib

Interactive Programming in C: modify your C code at runtime, see your changes on your application without restarting it.
Simulate your C programm has an interactive langage.

Inspired by:
* https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus
* https://github.com/mazbox/livecode-cplusplus
* http://nullprogram.com/blog/2014/12/23/

## Tutorial steps
* Step01: Load and run a function from a shared library.
* Step02: Auto-compile-and-load a function from a shared library. Modify the lib.c file and
when saving it, see the impact on the console.

## Prerequisite

Install: g++, gtkmm3. You need to install gtkmm3 for compiling the cpp file.
Indeed gtkmm3 has an archi-independent dynamic library loader. This will
simplify the code source for opening a dynamic library without doing different code
depending on Windows, Linux, OSX.

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

Note: For compiling the shared library for OS other than Linux and OSX, you may have to
adapt the script Makefile. Feel free to report an issue.

## To continue

* We load only C code. No C++ code yet !
* No segfault protection.
* Will be used for my [Forth C loader](https://github.com/Lecrapouille/SimTaDyn).