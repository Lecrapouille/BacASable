###############################################################################
# Compiler Warnings and Build Flags
#
# This module configures:
#   - Default build type (Release if unspecified)
#   - Per-build-type compiler flags (Debug / Release)
#   - Position Independent Code (PIC)
#   - target_set_warnings(): strict warning flags for a target
#
# Options:
#   ENABLE_WERROR      : Treat warnings as errors (-Werror). Default: OFF
#   CMAKE_BUILD_TYPE   : Build configuration. Default: Release
#                        Values: Debug, Release, MinSizeRel, RelWithDebInfo
#
# Usage:
#   cmake -S . -B build -DENABLE_WERROR=ON -DCMAKE_BUILD_TYPE=Debug
#
###############################################################################


###############################################################################
# ENABLE_WERROR - Treat warnings as errors
#
# Treat compiler warnings as errors (-Werror).
#
# Usage:
#   cmake -S . -B build -DENABLE_WERROR=ON
###############################################################################

option(ENABLE_WERROR "Treat compiler warnings as errors (-Werror)" OFF)


###############################################################################
# CMAKE_POSITION_INDEPENDENT_CODE - Position Independent Code (PIC)
#
# Required for shared libraries and recommended for static libraries
# that may be linked into shared libraries.
#
# Usage:
#   cmake -S . -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON
###############################################################################

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


###############################################################################
# Default Build Type
#
# If no build type is specified, default to Release.
# Available build types:
#   - Debug        : No optimization, debug symbols
#   - Release      : Full optimization, no debug
#   - MinSizeRel   : Optimize for size
#   - RelWithDebInfo: Optimization with debug info
###############################################################################

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()


###############################################################################
# Build-Type Compiler Flags
#
# CMake defaults: Debug="-g", Release="-O3 -DNDEBUG".
# We enhance Debug with -g3 -ggdb for maximum debug info (macro expansion, etc.)
# and explicit -O0 to prevent any optimization in debug builds.
###############################################################################

string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0 -g3 -ggdb")


###############################################################################
# Apply strict compiler warnings to a target (GCC / Clang, Linux).
# Flags are PRIVATE so they don't propagate to consumers.
#
# Arguments:
#   target_name - CMake target to configure
#
# Example:
#   add_library(mylib src/mylib.cpp)
#   target_set_warnings(mylib)
#
# Note: -Werror is conditionally added based on ENABLE_WERROR option.
###############################################################################

function(target_set_warnings target_name)

    # These warnings catch common programming errors and improve
    # code quality. They are applied to all builds (Debug & Release).
    target_compile_options(${target_name} PRIVATE
        # Basic warning sets
        -Wall                   # Standard warnings
        -Wextra                 # Additional warnings not in -Wall
        -Wpedantic              # ISO C++ strict compliance

        # Type safety warnings
        -Wshadow                # Variable shadowing
        -Wconversion            # Implicit type conversions
        -Wsign-conversion       # Signed/unsigned conversions

        # Code quality warnings
        -Wmissing-include-dirs  # Missing include directories
        -Wformat=2              # Printf format string issues
        -Wunused                # Unused code detection
        -Wcast-align            # Pointer alignment issues
        -Wnull-dereference      # Potential null dereferences

        # Diagnostic formatting
        -fdiagnostics-color=auto  # Colored compiler output
    )

    # Warnings as Errors (Optional)
    # When ENABLE_WERROR is ON, treat all warnings as compilation errors.
    # This is useful for CI pipelines to enforce warning-free code.
    if(ENABLE_WERROR)
        target_compile_options(${target_name} PRIVATE -Werror)
    endif()

endfunction()


###############################################################################
# Register Werror for configuration summary
###############################################################################

if(ENABLE_WERROR)
    list(APPEND _ENABLED_FEATURES "Werror")
    set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "")
endif()



# Link Time Optimization
if(ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
        list(APPEND _ENABLED_FEATURES "LTO")
        set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "")
        message(STATUS "LTO: Enabled")
    else()
        message(WARNING "LTO requested but not supported: ${lto_error}")
    endif()
endif()