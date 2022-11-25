## 2021 -- 2022 Quentin Quadrat quentin.quadrat@gmail.com
##
## This is free and unencumbered software released into the public domain.
##
## Anyone is free to copy, modify, publish, use, compile, sell, or
## distribute this software, either in source code form or as a compiled
## binary, for any purpose, commercial or non-commercial, and by any
## means.
##
## In jurisdictions that recognize copyright laws, the author or authors
## of this software dedicate any and all copyright interest in the
## software to the public domain. We make this dedication for the benefit
## of the public at large and to the detriment of our heirs and
## successors. We intend this dedication to be an overt act of
## relinquishment in perpetuity of all present and future rights to this
## software under copyright law.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
## IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
## OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
## ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
## OTHER DEALINGS IN THE SOFTWARE.
##
## For more information, please refer to <https://unlicense.org>

# Rename your binary and library or comment undesired one.
TARGET_BIN = MyApplication
TARGET_LIB = libmyapplication.so

# Destination pathes needed for the make install command.
DESTDIR ?=
PREFIX ?= /usr
BINDIR := $(DESTDIR)$(PREFIX)/bin
LIBDIR := $(DESTDIR)$(PREFIX)/lib
DATADIR := $(DESTDIR)$(PREFIX)/share/$(TARGET_BIN)/data

# Destination folder storing objects files, application and lib.
BUILD = build

# Compilation searching files
VPATH = $(BUILD) src src/foo src/bar

# Search C++ header files
INCLUDES = -Isrc -Isrc/foo

# C++11, C++14 ...
STANDARD=--std=c++14

# Desired compilation flags.
COMPIL_FLAGS = -Wall -Wextra -Wuninitialized -Wundef -Wunused  \
  -Wunused-result -Wunused-parameter -Wtype-limits -Wshadow    \
  -Wcast-align -Wcast-qual -Wconversion -Wfloat-equal          \
  -Wpointer-arith -Wswitch-enum -Wpacked -Wold-style-cast      \
  -Wdeprecated -Wvariadic-macros -Wvla -Wsign-conversion

# Undesired compilation flags.
COMPIL_FLAGS += -Wno-switch-enum -Wno-undef -Wno-unused-parameter \
  -Wno-old-style-cast -Wno-sign-conversion -Wcast-function-type

# Project compilation and linker flags.
CXXFLAGS += $(STANDARD) $(COMPIL_FLAGS)
LDFLAGS += -lpthread

# Example of external lib (SFML https://www.sfml-dev.org/index-fr.php)
# that uses pkg-config. Comment or rename the lib.
CXXFLAGS += `pkg-config --cflags sfml-graphics`
LDFLAGS += `pkg-config --libs sfml-graphics`

# Example of system lib (dlopen() ...) not using pkg-config
LDFLAGS += -ldl

## Pretty print the stack trace https://github.com/bombela/backward-cpp
## You can comment these lines if backward-cpp is not desired.
CXXFLAGS += -g -O0
LDFLAGS += -ldw
DEFINES += -DBACKWARD_HAS_DW=1
OBJS += backward.o

# Your project defines.
DEFINES += -DDATADIR=\"$(DATADIR)\"

# Generate header file dependencies needed to compile dependent files.
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD)/$*.Td
POSTCOMPILE = mv -f $(BUILD)/$*.Td $(BUILD)/$*.d

# Desired compiled files for the shared library.
LIB_OBJS += Foo.o Bar.o

# Desired compiled files for the application.
OBJS += main.o

# Verbosity control
ifeq ($(VERBOSE),1)
Q :=
else
Q := @
endif

# Compile the target
all: $(TARGET_BIN) $(TARGET_LIB)

# Link the target
$(TARGET_BIN): $(LIB_OBJS) $(OBJS)
	@echo "\033[0;32mLinking $@\033[0m"
	$(Q)cd $(BUILD) && $(CXX) $(INCLUDES) -o $(TARGET_BIN) $(LIB_OBJS) $(OBJS) $(LDFLAGS)

# Create the shared library
$(TARGET_LIB): $(LIB_OBJS)
	@echo "\033[0;32mCreate library $@\033[0m"
	$(Q)cd $(BUILD) && $(CXX) -shared -o $(TARGET_LIB) $(LIB_OBJS) $(LDFLAGS)

# Compile C++ source files
%.o : %.cpp $(BUILD)/%.d Makefile
	@echo "\033[0;32mCompiling $<\033[0m"
	$(Q)$(CXX) $(DEPFLAGS) -fPIC $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $(abspath $<) -o $(abspath $(BUILD)/$@)
	@$(POSTCOMPILE)

# Compile C source files
%.o : %.c $(BUILD)/%.d Makefile
	@echo "\033[0;32mCompiling $<\033[0m"
	$(Q)$(CXX) $(DEPFLAGS) -fPIC $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $(abspath $<) -o $(abspath $(BUILD)/$@)
	@$(POSTCOMPILE)

# Install the project
.PHONY: install
install: $(TARGET_BIN) $(TARGET_LIB)
	@echo "\033[0;32mInstalling $(TARGET_BIN)\033[0m"
	$(Q)mkdir -p $(BINDIR)
	$(Q)mkdir -p $(LIBDIR)
	$(Q)mkdir -p $(DATADIR)
	cp -r data $(DATADIR)/..
	cp $(BUILD)/$(TARGET_BIN) $(BINDIR)
	cp $(BUILD)/$(TARGET_LIB) $(LIBDIR)

# Do unit tests and code coverage
.PHONY: check
check:
	@echo "\033[0;32mCompiling unit tests\033[0m"
	$(Q)$(MAKE) -C tests check

# Create the documentation
.PHONY: doc
doc:
	@echo "\033[0;32mGenerating the API documentation\033[0m"
	$(Q)doxygen Doxyfile

# Compile LaTeX documentation
.PHONY: latex
latex:
	@echo "\033[0;32mCompiling the LaTeX documentation\033[0m"
	$(Q)cd doc && latex foo.tex && latex foo.tex && dvipdf foo.dvi

# Create the tarball
.PHONY: tarball
tarball:
	@echo "\033[0;32mCreating the tarball\033[0m"
	$(Q)./targz.sh $(PWD) $(TARGET_BIN)

# Delete compiled files
.PHONY: clean
clean:
	@echo "\033[0;32mCleaning\033[0m"
	$(Q)-rm -fr $(BUILD)

# Delete compiled files and backup files
.PHONY: veryclean
veryclean: clean
	@echo "\033[0;32mVery cleaning\033[0m"
	$(Q)-rm -fr *~ .*~
	$(Q)find src -name "*~" -print -delete
	$(Q)-rm -fr doc/html

# Create the directory before compiling sources
$(LIB_OBJS) $(OBJS): | $(BUILD)
$(BUILD):
	@mkdir -p $(BUILD)

# Create the dependency files
$(BUILD)/%.d: ;
.PRECIOUS: $(BUILD)/%.d

# Header file dependencies
-include $(patsubst %,$(BUILD)/%.d,$(basename $(LIB_OBJS) $(OBJS)))
