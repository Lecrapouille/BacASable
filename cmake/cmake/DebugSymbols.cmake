###############################################################################
# Debug Symbol Management
#
# This module provides:
#   - Split DWARF: Separate debug info into .dwo files (faster linking)
#   - Separated debug symbols at install time (Debug/RelWithDebInfo builds)
#
# Split DWARF (-gsplit-dwarf):
#   Debug information is written to separate .dwo files instead of embedding it
#   in .o files. This speeds up linking significantly because the linker doesn't
#   because the linker doesn't process debug info.
#
# Separated debug symbols (install-time):
#   For Debug/RelWithDebInfo builds, cmake --install automatically:
#     1. Extracts debug symbols to <dest>/.debug/<file>.debug
#     2. Strips the installed binary
#     3. Adds gnu-debuglink so GDB finds the .debug file automatically
#
#   The install tree looks like:
#     install/
#     ├── bin/
#     │   ├── application             # Stripped binary (like Release)
#     │   └── .debug/
#     │       └── application.debug   # Debug symbols only
#     └── lib/
#
#   GDB automatically discovers .debug/ subdirectories via gnu-debuglink.
#   For Release builds, use: cmake --install build --strip
#
# Usage:
#   cmake -S . -B build -DENABLE_SPLIT_DWARF=ON
###############################################################################


###############################################################################
# Compiling with separate debug information has the following benefits:
#   - Faster incremental linking (debug info is not processed by the linker)
#   - Smaller .o files
#   - Debug info is still available via .dwo files
###############################################################################

option(ENABLE_SPLIT_DWARF "Split debug symbols into .dwo files (-gsplit-dwarf)" OFF)

if(ENABLE_SPLIT_DWARF)
    message(STATUS "Split DWARF: Enabled (.dwo files)")
    add_enabled_feature("SplitDWARF")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
    message(STATUS "Separated debug symbols: Enabled (at install time)")
    add_enabled_feature("SeparateDebugSymbols")
endif()

###############################################################################
# target_enable_split_dwarf(<target>)
#
# Enable split DWARF for the specified target.
# Note: Only effective for Debug/RelWithDebInfo builds.
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
###############################################################################

function(target_enable_split_dwarf target_name)

    if(NOT ENABLE_SPLIT_DWARF)
        return()
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        message(WARNING "Split DWARF is not supported for Release or MinSizeRel builds")
        return()
    endif()

    message(STATUS "  Enabling split DWARF for ${target_name}")

    target_compile_options(${target_name} PRIVATE
        -gsplit-dwarf
        -gdwarf-4
    )

endfunction()


###############################################################################
# install_separated_debug_symbols(<target> <dest_subdir> <component>)
#
# At install time, for Debug/RelWithDebInfo builds:
#   1. Extract debug symbols to <prefix>/<dest_subdir>/.debug/<file>.debug
#   2. Strip the installed binary
#   3. Add gnu-debuglink for GDB auto-discovery
#
# The .debug/ subdirectory follows the standard Linux convention:
# GDB automatically searches <binary_dir>/.debug/ when resolving
# gnu-debuglink references, so no extra GDB configuration is needed.
#
# Skipped when cmake --install --strip is used (CMAKE_INSTALL_DO_STRIP),
# since the binary is already stripped by CMake and has no debug info left.
#
# Arguments:
#   target_name  - The CMake target
#   dest_subdir  - Relative install subdirectory (e.g., bin, lib)
#   component    - Install component (e.g., runtime, devel)
###############################################################################

function(install_separated_debug_symbols target_name dest_subdir component)

    if(NOT CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
        return()
    endif()

    find_program(DEBUG_OBJCOPY_EXE objcopy)
    find_program(DEBUG_STRIP_EXE strip)

    if(NOT DEBUG_OBJCOPY_EXE OR NOT DEBUG_STRIP_EXE)
        message(WARNING "objcopy or strip not found, cannot separate debug symbols for ${target_name}")
        return()
    endif()

    install(CODE "
        if(NOT CMAKE_INSTALL_DO_STRIP)
            set(_file \"\${CMAKE_INSTALL_PREFIX}/${dest_subdir}/$<TARGET_FILE_NAME:${target_name}>\")
            set(_debug_dir \"\${CMAKE_INSTALL_PREFIX}/${dest_subdir}/.debug\")
            set(_debug_file \"\${_debug_dir}/$<TARGET_FILE_NAME:${target_name}>.debug\")
            if(EXISTS \"\${_file}\")
                file(MAKE_DIRECTORY \"\${_debug_dir}\")
                execute_process(COMMAND \"${DEBUG_OBJCOPY_EXE}\" --only-keep-debug \"\${_file}\" \"\${_debug_file}\")
                execute_process(COMMAND \"${DEBUG_STRIP_EXE}\" --strip-debug --strip-unneeded \"\${_file}\")
                execute_process(COMMAND \"${DEBUG_OBJCOPY_EXE}\" \"--add-gnu-debuglink=\${_debug_file}\" \"\${_file}\")
                message(STATUS \"Separated debug symbols: \${_debug_file}\")
            endif()
        endif()
    " COMPONENT ${component})

endfunction()