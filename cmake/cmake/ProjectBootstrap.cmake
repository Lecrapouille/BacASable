# ============================================================================
# ProjectBootstrap.cmake - Project-Wide Configuration
# ============================================================================
#
# Single entry point included from the root CMakeLists.txt right after
# project(). It declares all build options, sets global CMake variables,
# and includes every feature module in dependency order.
#
# Include order:
#   1. C++ standard & global CMake settings
#   2. Module path registration
#   3. Build options (declared here or in each module)
#   4. Core modules (warnings, debug symbols, PCH)
#   5. Optional feature modules (LTO, sanitizers, coverage, docs)
#   6. Third-party dependencies (backward-cpp)
#   7. Testing infrastructure
#   8. Project utilities (add_module_library / add_module_executable)
#   9. Configuration summary
#
# ============================================================================


# C++ Standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # Fail if the compiler doesn't support it
set(CMAKE_CXX_EXTENSIONS OFF)          # Disable compiler-specific extensions (e.g. GNU)

# Generate compile_commands.json for IDEs (CLion, VS Code) and tools (clang-tidy, clangd).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ============================================================================
# Module Path
# ============================================================================
# Register cmake/ so that include() can find our custom modules.
# Must be set before any include() calls below.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# ============================================================================
# Build Options
# ============================================================================
# Options that don't belong to a specific module are declared here.
# Module-specific options (ENABLE_WERROR, ENABLE_SPLIT_DWARF, …) are
# declared inside their respective .cmake files.

# ----------------------------------------------------------------------------
# ENABLE_LTO - Link Time Optimization
# ----------------------------------------------------------------------------
# Whole-program optimization at link time: smaller and faster binaries,
# but longer link times. Only applied in Release builds.
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON

option(ENABLE_LTO "Enable Link Time Optimization for Release builds" OFF)




# ----------------------------------------------------------------------------
# ENABLE_DOXYGEN - API Documentation
# ----------------------------------------------------------------------------
# Generate HTML documentation from source comments with Doxygen.
#
# Usage:
#   cmake -S . -B build -DENABLE_DOXYGEN=ON
#   cmake --build build --target docs

option(ENABLE_DOXYGEN "Enable Doxygen documentation generation" OFF)


# List of enabled features, populated by each module.
# Used for the configuration summary at the end.
set(_ENABLED_FEATURES "" CACHE INTERNAL "List of enabled features for summary")


# Pretty-printing functions for status messages
include(PrettyPrint)

# Default build type, flags, target_set_warnings()
include(CompilerWarnings) 
# ENABLE_SPLIT_DWARF, ENABLE_STRIP, symbol helpers 
include(DebugSymbols)

# Global precompiled header, target_use_global_pch()
include(PCH)                




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


# Apply runtime sanitizers (ASAN, UBSAN, TSAN) to the specified target.
include(Sanitizers)

# Instrument code for coverage analysis with gcov/lcov.
include(Coverage)

# Generate API documentation with Doxygen.
include(Doxygen)

# Conan package manager integration
include(Conan)

# Beautiful stack traces on crashes.
include(FetchBackwardCpp)

# Configure unit testing with GoogleTest framework
include(Testing)

# Project Utilities: Provides add_module_library() and add_module_executable().
include(Utilities)


# ============================================================================
# Configuration Summary
# ============================================================================

# Project info box
print_section_start("${PROJECT_NAME}")
print_section_line("Version" "${PROJECT_VERSION}")
print_section_line("Description" "${PROJECT_DESCRIPTION}")
print_section_line("Source" "${CMAKE_SOURCE_DIR}")
print_section_line("Build" "${CMAKE_BINARY_DIR}")
print_section_end()

# Configuration summary box
print_section_start("Configuration")

print_section_line("Compiler" "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
print_section_line("Standard" "C++${CMAKE_CXX_STANDARD}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    print_section_line("Build Type" "${CMAKE_BUILD_TYPE} (no optimization)")
else()
    print_section_line("Build Type" "${CMAKE_BUILD_TYPE}")
endif()

# Features from _ENABLED_FEATURES (populated by each module)
if(_ENABLED_FEATURES)
    list(JOIN _ENABLED_FEATURES ", " _features_str)
    print_section_line("Features" "${_features_str}")
else()
    print_section_line("Features" "None")
endif()

# Sanitizers warning
if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
    print_section_line("Warning" "Sanitizers active - slower execution")
endif()

print_section_end()
