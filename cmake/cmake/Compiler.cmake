###############################################################################
# Compiler & Build Configuration
#
# This module configures the compiler and build system:
#   - Build type (Release if unspecified)
#   - Per-build-type compiler flags (Debug / Release)
#   - Position Independent Code (PIC)
#   - Link Time Optimization (LTO)
#   - Strict warning flags per target
#
# Options:
#   ENABLE_WERROR      : Treat warnings as errors (-Werror). Default: OFF
#   CMAKE_BUILD_TYPE   : Build configuration. Default: Release
#                        Values: Debug, Release, MinSizeRel, RelWithDebInfo
# Usage:
#   cmake -S . -B build -DENABLE_WERROR=ON -DCMAKE_BUILD_TYPE=Debug
###############################################################################


###############################################################################
# Treat compiler warnings as errors
#
# Usage:
#   cmake -S . -B build -DENABLE_WERROR=ON
###############################################################################

option(ENABLE_WERROR "Treat compiler warnings as errors (-Werror)" OFF)

if(ENABLE_WERROR)
    message(STATUS "Werror: Enabled")
    add_enabled_feature("Werror")
endif()


###############################################################################
# Position Independent Code (PIC)
#
# Required for shared libraries and recommended for static libraries that may
# be linked into shared libraries.
#
# Usage:
#   cmake -S . -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON
###############################################################################

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_POSITION_INDEPENDENT_CODE)
    message(STATUS "PIC: Enabled")
    add_enabled_feature("PIC")
endif()


###############################################################################
# Available build types:
#   - Debug          : No optimization, debug symbols
#   - Release        : Full optimization, no debug
#   - MinSizeRel     : Optimize for size
#   - RelWithDebInfo : Optimization with debug info
#
# If no build type is specified, default to Release.
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
# Set compiler warnings to a target (GCC / Clang, Linux) for all builds (Debug
# and Release). Note: Flags are PRIVATE so they don't propagate to consumers.
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

    # Treat warnings as errors (Optional)
    if(ENABLE_WERROR)
        target_compile_options(${target_name} PRIVATE -Werror)
    endif()

endfunction()


###############################################################################
# Link Time Optimization
#
# Whole-program optimization at link time: smaller and faster binaries,
# but longer link times. Only applied in Release builds.
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
###############################################################################

option(ENABLE_LTO "Enable Link Time Optimization (for Release builds)" OFF)

if(ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
        message(STATUS "LTO: Enabled")
        add_enabled_feature("LTO")
    else()
        message(WARNING "LTO requested but not supported: ${lto_error}")
    endif()
endif()