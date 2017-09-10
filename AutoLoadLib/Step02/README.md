Compilation for OS X:
make

Note 01: For compiling the shared library for other OS, you have to adapt the Makefile.
Note 02: you need to install gtkmm3 (brew install gtkmm3) for compiling module.cpp.

Execution:
./module

With any text editor, modify the lib.c, you should see the modifications on the console.
Note 03: module.cpp is not protected against segfault produced by developer mistakes in lib.c. This will be done for the next step.
