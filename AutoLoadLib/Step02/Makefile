all: module

### Runner
.PHONY: module
module:
	g++ -W -Wall --std=c++11 `pkg-config --cflags --libs gtkmm-3.0` module.cpp -o module

### LIB
.PHONY: lib
lib:
	gcc -c -W -Wall lib.c
	gcc -dynamiclib -undefined suppress -flat_namespace lib.o -o livecode.dylib

clean:
	rm -fr *~ *.o *.dylib module
