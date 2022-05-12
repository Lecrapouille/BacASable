TARGET_BIN = Syracuse

# Detect the operating system: Unix, OSX and Windows
ifeq ($(OS),Windows_NT)
ARCHI := Windows
else
ARCHI := $(shell uname -s)
endif

# Compilation searching files
BUILD = build
VPATH = $(BUILD) src src/OpenGL
INCLUDES = -Isrc -Isrc/OpenGL

# C++14 (only because of std::make_unique not present in C++11)
STANDARD=--std=c++11

# Compilation flags
COMPIL_FLAGS = -Wall -Wextra -Wuninitialized -Wundef -Wunused       \
  -Wunused-result -Wunused-parameter -Wtype-limits                  \
  -Wcast-align -Wcast-qual -Wconversion -Wfloat-equal               \
  -Wpointer-arith -Wswitch-enum -pedantic -Wpacked -Wold-style-cast \
  -Wdeprecated -Wvariadic-macros -Wvla -Wsign-conversion
COMPIL_FLAGS += -Wno-switch-enum -Wno-undef -Wno-unused-parameter \
  -Wno-old-style-cast -Wno-sign-conversion

# OpenGL: glfw and glew libraries
ifeq ($(ARCHI),Darwin)
INCLUDES += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
LDFLAGS += -framework OpenGL -framework Cocoa
LDFLAGS += -framework IOKit -framework CoreVideo
LDFLAGS += -L/opt/homebrew/lib
LDFLAGS += -lGLEW -lglfw
else ifeq ($(ARCHI),Linux)
CXXFLAGS += `pkg-config --cflags glew --static glfw3`
LDFLAGS += `pkg-config --libs glew --static glfw3`
else
$(error Unknown architecture)
endif

# Project flags
CXXFLAGS += $(STANDARD) $(COMPIL_FLAGS) -DCHECK_OPENGL

# Header file dependencies
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD)/$*.Td
POSTCOMPILE = mv -f $(BUILD)/$*.Td $(BUILD)/$*.d

# Desired compiled files
OBJS = GLWindow.o GLCore.o Application.o main.o

# Verbosity control
ifeq ($(VERBOSE),1)
Q :=
else
Q := @
endif

# Compile the target
all: $(TARGET_BIN) $(TARGET_LIB)

# Link the target
$(TARGET_BIN): $(OBJS)
	@echo "Linking $@"
	$(Q)cd $(BUILD) && $(CXX) $(INCLUDES) -o $(TARGET_BIN) $(OBJS) $(LDFLAGS)

# Create the shared library
$(TARGET_LIB): $(LIB_OBJS)
	@echo "Library $@"
	$(Q)cd $(BUILD) && $(CXX) -shared -o $(TARGET_LIB) $(LIB_OBJS) $(LDFLAGS)

# Compile C++ source files
%.o : %.cpp $(BUILD)/%.d Makefile
	@echo "Compiling $<"
	$(Q)$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $(abspath $<) -o $(abspath $(BUILD)/$@)
	@$(POSTCOMPILE)

# Compile C source files
%.o : %.c $(BUILD)/%.d Makefile
	@echo "Compiling $<"
	$(Q)$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $(abspath $<) -o $(abspath $(BUILD)/$@)
	@$(POSTCOMPILE)

# Install the project
.PHONY: install
install: $(TARGET_BIN) $(TARGET_LIB)
	@echo "Installing $(TARGET_BIN)"
	$(Q)mkdir -p $(BINDIR)
	$(Q)mkdir -p $(LIBDIR)
	$(Q)mkdir -p $(DATADIR)
	cp $(BUILD)/$(TARGET_BIN) $(BINDIR)
	cp $(BUILD)/$(TARGET_LIB) $(LIBDIR)
	cp -r data $(DATADIR)/..

# Do unit tests and code coverage
.PHONY: check
check:
	@echo "Compiling unit tests"
	$(Q)$(MAKE) -C tests check

# Create the documentation
.PHONY: doc
doc:
	$(Q)doxygen Doxyfile

# Create the tarball
.PHONY: tarball
tarball:
	$(Q)./.targz.sh $(PWD) $(TARGET_BIN)

# Compile LaTeX documentation
.PHONY: latex
latex:
	$(Q)cd doc/IA/ && latex IA.tex && latex IA.tex && dvipdf IA.dvi

# Delete compiled files
.PHONY: clean
clean:
	$(Q)-rm -fr $(BUILD)

# Delete compiled files and backup files
.PHONY: veryclean
veryclean: clean
	$(Q)-rm -fr *~ .*~
	$(Q)find src -name "*~" -print -delete
	$(Q)-rm -fr doc/html

# Create the directory before compiling sources
$(OBJS): | $(BUILD)
$(BUILD):
	@mkdir -p $(BUILD)

# Create the dependency files
$(BUILD)/%.d: ;
.PRECIOUS: $(BUILD)/%.d

# Header file dependencies
-include $(patsubst %,$(BUILD)/%.d,$(basename $(OBJS)))
