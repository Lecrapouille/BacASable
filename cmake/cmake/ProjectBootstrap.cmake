###############################################################################
# ProjectBootstrap.cmake - Project-Wide Configuration
#
# Single entry point included from the root CMakeLists.txt right after
# project(). It declares all build options, sets global CMake variables,
# and includes every feature module in dependency order.
#
# Include order:
#   1. C++ standard & global CMake settings
#   2. Module path registration
#   3. Core modules (compiler, debug symbols, PCH)
#   4. Optional feature modules (sanitizers, coverage, docs)
#   5. Third-party dependencies (Conan, backward-cpp)
#   6. Testing infrastructure (GoogleTest, CTest)
#   7. Module API (add_module_library / add_module_executable)
#   8. Configuration summary
#
###############################################################################


###############################################################################
# C++ Standard
###############################################################################

set(CMAKE_CXX_STANDARD 20)             # Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # Fail if the compiler doesn't support it
set(CMAKE_CXX_EXTENSIONS OFF)          # Disable compiler-specific extensions (e.g. GNU)

###############################################################################
# Generate compile_commands.json for IDEs (CLion, VS Code) and tools
# (clang-tidy, clangd).
###############################################################################

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################################################################
# Register cmake/ so that include() can find our custom modules.
# Must be set before any include() calls below.
###############################################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

###############################################################################
# Feature tracking list
# Populated by each module for the summary box
###############################################################################

set(_ENABLED_FEATURES "" CACHE INTERNAL "List of enabled features for summary")

###############################################################################
# add_enabled_feature(<feature> [<feature> ...])
#
# Register one or more features for the configuration summary.
# Call from any module that enables optional functionality.
###############################################################################

function(add_enabled_feature)
    list(APPEND _ENABLED_FEATURES ${ARGV})
    set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "" FORCE)
endfunction()

###############################################################################
# CMake Modules
###############################################################################

include(PrettyPrint)        # Pretty-printing functions for status messages
include(Conan)              # Enable Conan package manager + find_conan_package()
include(Compiler)           # Enable compiler flags, LTO, target_set_warnings()
include(DebugSymbols)       # Split DWARF, separated debug symbols at install time
include(Stacktrace)         # Enable backward-cpp crash stack traces
include(PCH)                # Enable global precompiled header, target_configure_pch()
include(Sanitizers)         # Enable sanitizers (ASAN, UBSAN, TSAN)
include(Coverage)           # Enable code coverage instrumentation (gcov/lcov)
include(Doxygen)            # Enable Doxygen documentation generation
include(Testing)            # Enable testing infrastructure (GoogleTest, CTest)
include(ModuleAPI)          # Enable module API (add_module_library(), add_module_executable())
include(Summary)            # Enable summary box

###############################################################################
# Configuration Summary (automatic)
###############################################################################

_summary_init()