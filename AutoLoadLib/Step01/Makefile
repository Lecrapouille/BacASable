################################################################################
###
TARGET=autoloader
DYLIB=liblivecode

MAKEFLAGS += --no-print-directory

################################################################################
### Detect the operating system: Unix, OSX and Windows
ifeq ($(OS),Windows_NT)
ARCHI := Windows
else
ARCHI := $(shell uname -s)
endif

################################################################################
###
all: $(TARGET) lib

################################################################################
### The dynamic library auto-loader
$(TARGET):
	@g++ -W -Wall --std=c++11 `pkg-config --cflags --libs gtkmm-3.0` $(TARGET).cpp -o $(TARGET)

################################################################################
### Dynamic library
.PHONY: lib
lib:
ifeq ($(ARCHI),Darwin)
	@gcc -c -W -Wall lib.c
	@gcc -dynamiclib -undefined suppress -flat_namespace lib.o -o $(DYLIB).dylib
else ifeq ($(ARCHI),Linux)
	@gcc -c -W -Wall -fpic lib.c
	@gcc -shared lib.o -o $(DYLIB).so
else ifeq ($(OS),Windows_NT)
	@gcc -c -W -Wall -fpic lib.c
	@gcc -shared lib.o -o $(DYLIB).dll -Wl,--out-implib,$(DYLIB).a
else
	error "I dunno how to compile dynamic lib with this architecture"
endif
	@rm -fr lib.o

################################################################################
### Launch the dynamic library auto-loader
.PHONY: launch
launch: $(TARGET) lib
	@./$(TARGET)

################################################################################
###
.PHONY: clean
clean:
	@rm -fr *~ *.o *.dylib *.dll *.so $(TARGET)
