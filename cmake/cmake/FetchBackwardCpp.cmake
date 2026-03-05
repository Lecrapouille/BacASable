# ============================================================================
# FetchBackwardCpp.cmake - Stack Trace Library
# ============================================================================
#
# This module fetches backward-cpp, a library for beautiful stack traces
# on crashes and exceptions.
#
# backward-cpp provides:
#   - Human-readable stack traces on segfaults/crashes
#   - Source code snippets in traces (requires debug symbols)
#   - Works with GDB, addr2line, or libdw/libdwarf
#
# Prerequisites (for full functionality):
#   sudo apt install binutils-dev      # For addr2line backend
#   sudo apt install libdw-dev         # For libdw backend (recommended)
#   sudo apt install libdwarf-dev      # For libdwarf backend
#
# Usage in your module:
#   target_link_libraries(myapp PRIVATE Backward::Interface)
#   # Or for automatic signal handling:
#   target_link_libraries(myapp PRIVATE Backward::Backward)
#
# The Backward::Backward target automatically:
#   - Registers signal handlers for SIGSEGV, SIGABRT, etc.
#   - Prints stack traces on crashes
#
# ============================================================================

option(FETCH_BACKWARD_CPP "Fetch backward-cpp" ON)

if(FETCH_BACKWARD_CPP)

include(FetchContent)

print_box("backward-cpp Stack Traces")

# ----------------------------------------------------------------------------
# Declare backward-cpp Dependency
# ----------------------------------------------------------------------------

FetchContent_Declare(
    backward
    GIT_REPOSITORY https://github.com/bombela/backward-cpp.git
    GIT_TAG        v1.6
)


# ----------------------------------------------------------------------------
# Configure backward-cpp
# ----------------------------------------------------------------------------
# BACKWARD_ENABLE_STACK_WALKING_UNWIND: Use _Unwind_Backtrace (most compatible)
# BACKWARD_HAS_DW: Use libdw for DWARF debug info (recommended on Linux)
# BACKWARD_HAS_DWARF: Use libdwarf as alternative
# BACKWARD_HAS_BFD: Use libbfd (binutils) for debug info

# Try to find libdw (preferred)
find_path(LIBDW_INCLUDE_DIR NAMES "elfutils/libdw.h")
find_library(LIBDW_LIBRARY NAMES dw)

if(LIBDW_INCLUDE_DIR AND LIBDW_LIBRARY)
    set(BACKWARD_HAS_DW 1 CACHE BOOL "Use libdw for stack traces")
    message(STATUS "backward-cpp: Using libdw backend")
else()
    # Fallback to addr2line
    find_program(ADDR2LINE addr2line)
    if(ADDR2LINE)
        message(STATUS "backward-cpp: Using addr2line backend")
    else()
        message(STATUS "backward-cpp: Basic mode (install libdw-dev for better traces)")
    endif()
endif()


# ----------------------------------------------------------------------------
# Make backward-cpp Available
# ----------------------------------------------------------------------------

FetchContent_MakeAvailable(backward)


# ----------------------------------------------------------------------------
# Available Targets
# ----------------------------------------------------------------------------
# After FetchContent_MakeAvailable, backward-cpp provides:
#   - Backward::Backward   : Full library with automatic signal handlers
#   - Backward::Interface  : Header-only interface (if available)
#
# Note: Target names may vary by backward-cpp version.
# Check available targets with: get_property(_targets GLOBAL PROPERTY TARGETS)

if(TARGET Backward::Backward)
    message(STATUS "backward-cpp: Target Backward::Backward available")
elseif(TARGET backward)
    add_library(Backward::Backward ALIAS backward)
    message(STATUS "backward-cpp: Created alias Backward::Backward -> backward")
endif()


message(STATUS "backward-cpp: Available targets - Backward::Interface, Backward::Backward")

endif()