################################################################################
###
TARGET=Autoloader
DYLIB=liblivecode

################################################################################
### You can replace by your favorite text editor such gedit, emacs, vi.
TEXT_EDITOR=gedit
# xdg-open

################################################################################
###
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
### Compile the dynamic library auto-loader
$(TARGET): $(TARGET).cpp
	g++ -Wall -Wextra --std=c++14 $(TARGET).cpp -o $(TARGET) `pkg-config --cflags --libs gtkmm-3.0` -ldl

################################################################################
### Compile the dynamic library
.PHONY: lib
lib: lib.cpp
ifeq ($(ARCHI),Darwin)
	g++ --std=c++14 -c -Wall -Wextra lib.cpp
	g++ --std=c++14 -dynamiclib -undefined suppress -flat_namespace lib.o -o $(DYLIB).dylib
else ifeq ($(ARCHI),Linux)
	g++ --std=c++14 -c -Wall -Wextra -fpic lib.cpp
	g++ --std=c++14 -shared lib.o -o $(DYLIB).so
else ifeq ($(OS),Windows_NT)
	g++ --std=c++14 -c -Wall -Wextra -fpic lib.cpp
	g++ --std=c++14 -shared lib.o -o $(DYLIB).dll -Wl,--out-implib,$(DYLIB).a
else
	error "I dunno how to compile dynamic lib with this architecture"
endif
	@rm -fr lib.o

################################################################################
### Launch the dynamic library auto-loader and launch on the text editor on the
### observed file.
.PHONY: launch
launch: $(TARGET) lib
	$(TEXT_EDITOR) lib.cpp &
	./$(TARGET)

################################################################################
###
.PHONY: clean
clean:
	rm -fr *~ *.o *.dylib *.dll *.so $(TARGET)
