###############################################################################
# Sanitizers.cmake - Runtime Sanitizer Configuration
###############################################################################
#
# Provides target_enable_sanitizers() to apply runtime sanitizers.
# ASAN and TSAN cannot be used together.
#
# Options:
#   ENABLE_ASAN      : Enable AddressSanitizer (memory error detection). Default: OFF
#   ENABLE_UBSAN     : Enable UndefinedBehaviorSanitizer. Default: OFF
#   ENABLE_TSAN      : Enable ThreadSanitizer (data race detection). Default: OFF
#
# Usage:
#   cmake -S . -B build -DENABLE_ASAN=ON
#   cmake -S . -B build -DENABLE_UBSAN=ON
#   cmake -S . -B build -DENABLE_TSAN=ON
#
###############################################################################


###############################################################################
# ENABLE_ASAN - AddressSanitizer (memory error detection)
#
# Detect memory errors: buffer overflows, use-after-free, double-free, leaks.
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
###############################################################################
option(ENABLE_ASAN "Enable AddressSanitizer (memory error detection)" OFF)


###############################################################################
# ENABLE_UBSAN - UndefinedBehaviorSanitizer (undefined behavior detection)
#
# Detect undefined behavior: integer overflow, null dereference, etc.
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_UBSAN=ON
###############################################################################
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)


###############################################################################
# ENABLE_TSAN - ThreadSanitizer (data race detection)
#
# Detect threading issues: data races, deadlocks.
# Cannot be used together with ASAN.
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
###############################################################################
option(ENABLE_TSAN "Enable ThreadSanitizer (data race detection)" OFF)

###############################################################################
# Validation: incompatible sanitizer combinations
###############################################################################
if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR
        "ENABLE_ASAN and ENABLE_TSAN cannot be enabled simultaneously.\n"
        "AddressSanitizer and ThreadSanitizer have conflicting instrumentation.\n"
        "Please disable one of them."
    )
endif()

if((ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN) AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING
        "Sanitizers are enabled in Release build.\n"
        "For better error detection, use Debug build: -DCMAKE_BUILD_TYPE=Debug"
    )
endif()


###############################################################################
# target_enable_sanitizers(<target>)
#
# Apply enabled sanitizers (ASAN, UBSAN, TSAN) to the specified target.
#
# Arguments:
#   target_name - The CMake target to instrument
#
# Example:
#   add_library(mylib src/mylib.cpp)
#   target_enable_sanitizers(mylib)
#
# Sanitizer flags:
#   -fsanitize=address : Enable AddressSanitizer
#   -fsanitize=undefined : Enable UndefinedBehaviorSanitizer
#   -fsanitize=thread : Enable ThreadSanitizer
###############################################################################
function(target_enable_sanitizers target_name)
    set(_sanitizer_flags "")
    set(_sanitizer_names "")

    if(ENABLE_ASAN)
        list(APPEND _sanitizer_flags "-fsanitize=address" "-fno-omit-frame-pointer")
        list(APPEND _sanitizer_names "ASAN")
    endif()

    if(ENABLE_UBSAN)
        list(APPEND _sanitizer_flags "-fsanitize=undefined")
        list(APPEND _sanitizer_names "UBSAN")
    endif()

    if(ENABLE_TSAN)
        list(APPEND _sanitizer_flags "-fsanitize=thread")
        list(APPEND _sanitizer_names "TSAN")
    endif()

    if(_sanitizer_flags)
        message(STATUS "  Enabling sanitizers for ${target_name}: ${_sanitizer_names}")
        target_compile_options(${target_name} PRIVATE ${_sanitizer_flags})
        target_link_options(${target_name} PRIVATE ${_sanitizer_flags})
    endif()
endfunction()


# Register for configuration summary
set(_sanitizer_features "")
if(ENABLE_ASAN)
    list(APPEND _sanitizer_features "AddressSanitizer")
endif()
if(ENABLE_UBSAN)
    list(APPEND _sanitizer_features "UBSanitizer")
endif()
if(ENABLE_TSAN)
    list(APPEND _sanitizer_features "ThreadSanitizer")
endif()
if(_sanitizer_features)
    list(APPEND _ENABLED_FEATURES ${_sanitizer_features})
    set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "")
endif()
