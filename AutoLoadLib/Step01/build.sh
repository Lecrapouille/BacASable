#!/bin/bash

rm -fr *.o *.dylib module *~

##
gcc -c -W -Wall lib.c
gcc -dynamiclib -undefined suppress -flat_namespace lib.o -o livecode.dylib
rm -fr lib.o

##
g++ -W -Wall --std=c++11 `pkg-config --cflags --libs gtkmm-3.0` module.cpp -o module
