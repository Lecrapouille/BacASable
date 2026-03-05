###############################################################################
# DebugSymbols.cmake - Debug Symbol Management
###############################################################################
#
# This module provides functions for managing debug symbols:
#   - Split DWARF: Separate debug info into .dwo files (faster linking)
#   - Strip symbols: Remove symbols from release binaries (smaller size)
#
# Split DWARF (-gsplit-dwarf):
#   Debug information is written to separate .dwo files instead of
#   embedding it in .o files. This speeds up linking significantly
#   because the linker doesn't process debug info.
#
# Strip symbols:
#   Remove symbol tables from release binaries to reduce size.
#   Debug symbols can be extracted to separate .debug files for
#   post-mortem debugging.
#
# Usage:
#   cmake -S . -B build -DENABLE_SPLIT_DWARF=ON   # Split debug symbols
#   cmake -S . -B build -DENABLE_STRIP=ON         # Strip release binaries
#
###############################################################################


###############################################################################
# ENABLE_LTO - Link Time Optimization
#
# Whole-program optimization at link time: smaller and faster binaries,
# but longer link times. Only applied in Release builds.
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
###############################################################################

option(ENABLE_LTO "Enable Link Time Optimization for Release builds" OFF)

###############################################################################
# ENABLE_SPLIT_DWARF - Separate Debug Information
#
# Compile with -gsplit-dwarf to generate .dwo files.
# Benefits:
#   - Faster incremental linking (debug info not processed by linker)
#   - Smaller .o files
#   - Debug info still available via .dwo files
#
# To debug: GDB automatically finds .dwo files if they're in the same
# directory as the .o files.
#
# Default: OFF
###############################################################################

option(ENABLE_SPLIT_DWARF "Split debug symbols into .dwo files (-gsplit-dwarf)" OFF)


###############################################################################
# ENABLE_STRIP - Strip Symbols in Release
#
# Strip symbol tables from release binaries.
# Creates smaller binaries but removes ability to debug without
# separate debug files.
#
# Default: OFF
###############################################################################

option(ENABLE_STRIP "Strip symbols from release binaries" OFF)


###############################################################################
# target_enable_split_dwarf(<target>)
###############################################################################
# Enable split DWARF for the specified target.
#
# Arguments:
#   target_name - The CMake target to configure
#
# Example:
#   add_executable(myapp main.cpp)
#   target_enable_split_dwarf(myapp)
#
# Generated files:
#   - build/CMakeFiles/myapp.dir/src/main.cpp.o.dwo
#
# Note: Only effective for Debug/RelWithDebInfo builds.
###############################################################################

function(target_enable_split_dwarf target_name)

    if(NOT ENABLE_SPLIT_DWARF)
        return()
    endif()

    # Only apply to Debug or RelWithDebInfo
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        return()
    endif()

    message(STATUS "  Enabling split DWARF for ${target_name}")

    # -gsplit-dwarf: Generate .dwo files for debug info
    # -gdwarf-4: Use DWARF v4 format (widely supported)
    target_compile_options(${target_name} PRIVATE
        -gsplit-dwarf
        -gdwarf-4
    )

endfunction()


###############################################################################
# target_strip_symbols(<target>)
###############################################################################
# Strip symbols from the target binary after linking.
# Only applies to Release builds.
#
# Arguments:
#   target_name - The CMake target to strip
#
# Example:
#   add_executable(myapp main.cpp)
#   target_strip_symbols(myapp)
#
# This creates a post-build command that runs 'strip' on the binary.
###############################################################################

function(target_strip_symbols target_name)

    if(NOT ENABLE_STRIP)
        return()
    endif()

    # Only strip in Release mode
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        return()
    endif()

    find_program(STRIP_EXECUTABLE strip)

    if(NOT STRIP_EXECUTABLE)
        message(WARNING "strip not found, cannot strip ${target_name}")
        return()
    endif()

    message(STATUS "  Enabling symbol stripping for ${target_name}")

    # Add post-build strip command
    add_custom_command(TARGET ${target_name}
        POST_BUILD
        COMMAND ${STRIP_EXECUTABLE} --strip-all $<TARGET_FILE:${target_name}>
        COMMENT "Stripping symbols from ${target_name}"
        VERBATIM
    )

endfunction()


###############################################################################
# target_extract_debug_symbols(<target>)
###############################################################################
# Extract debug symbols to a separate .debug file before stripping.
# This allows debugging release builds using the separate debug file.
#
# Arguments:
#   target_name - The CMake target to process
#
# Example:
#   add_executable(myapp main.cpp)
#   target_extract_debug_symbols(myapp)
#
# Generated files:
#   - myapp.debug  (debug symbols)
#   - myapp        (stripped binary)
#
# To debug with extracted symbols:
#   gdb -s myapp.debug myapp
###############################################################################

function(target_extract_debug_symbols target_name)

    if(NOT ENABLE_STRIP)
        return()
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        return()
    endif()

    find_program(OBJCOPY_EXECUTABLE objcopy)
    find_program(STRIP_EXECUTABLE strip)

    if(NOT OBJCOPY_EXECUTABLE OR NOT STRIP_EXECUTABLE)
        message(WARNING "objcopy or strip not found, cannot extract debug symbols")
        return()
    endif()

    message(STATUS "  Enabling debug symbol extraction for ${target_name}")

    # Extract debug symbols to .debug file, then strip original
    add_custom_command(TARGET ${target_name}
        POST_BUILD
        # Extract debug symbols
        COMMAND ${OBJCOPY_EXECUTABLE}
            --only-keep-debug
            $<TARGET_FILE:${target_name}>
            $<TARGET_FILE:${target_name}>.debug
        # Strip the binary
        COMMAND ${STRIP_EXECUTABLE}
            --strip-all
            $<TARGET_FILE:${target_name}>
        # Add debug link to stripped binary (allows GDB to find .debug file)
        COMMAND ${OBJCOPY_EXECUTABLE}
            --add-gnu-debuglink=$<TARGET_FILE:${target_name}>.debug
            $<TARGET_FILE:${target_name}>
        COMMENT "Extracting debug symbols from ${target_name}"
        VERBATIM
    )

endfunction()


# ############################################################################
#                         GLOBAL CONFIGURATION
# ############################################################################


# Display status and register for configuration summary
if(ENABLE_SPLIT_DWARF)
    message(STATUS "Split DWARF: Enabled (.dwo files)")
    list(APPEND _ENABLED_FEATURES "SplitDWARF")
    set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "")
endif()

if(ENABLE_STRIP)
    message(STATUS "Symbol stripping: Enabled (Release only)")
    list(APPEND _ENABLED_FEATURES "Strip")
    set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "")
endif()
