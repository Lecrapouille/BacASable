###############################################################################
# Summary.cmake - Configuration summary box
#
# Provides an automatic summary box for the project configuration.
# Modules are collected during add_module_* and printed at end of configure.
# Generates a dependency graph (.dot) for public (green) and private (red) deps.
#
# Everything is automatic: include(Summary) defines the functions, and
# _summary_init() (called from ProjectBootstrap.cmake) prints the project
# box immediately and defers the configuration summary + dependency graph
# to the end of the configure step.
#
# Usage:
#   include(Summary)          # done by ProjectBootstrap.cmake
#   _summary_init()           # done by ProjectBootstrap.cmake
#
###############################################################################

# Global properties for module tracking (avoids ";" escaping in dependency lists)
define_property(GLOBAL PROPERTY PROJECT_MODULE_NAMES
    BRIEF_DOCS "Ordered list of module names"
    FULL_DOCS "Semicolon-separated"
)

###############################################################################
# summary_register_module(<name> <type> <public_deps> <private_deps> [VERSION ver])
#
# Called by add_module_library / add_module_executable to register modules.
# <type>: static, shared, executable
# <public_deps>: semicolon-separated list
# <private_deps>: semicolon-separated list
# VERSION: optional module version (for dot graph edge labels)
###############################################################################

function(summary_register_module name type public_deps private_deps)
    cmake_parse_arguments(ARG "" "VERSION" "" ${ARGN})
    set_property(GLOBAL APPEND PROPERTY PROJECT_MODULE_NAMES "${name}")
    set_property(GLOBAL PROPERTY "PROJECT_MODULE_${name}_TYPE" "${type}")
    set_property(GLOBAL PROPERTY "PROJECT_MODULE_${name}_PUBLIC" "${public_deps}")
    set_property(GLOBAL PROPERTY "PROJECT_MODULE_${name}_PRIVATE" "${private_deps}")
    if(ARG_VERSION)
        set_property(GLOBAL PROPERTY "PROJECT_MODULE_${name}_VERSION" "${ARG_VERSION}")
    endif()
endfunction()

###############################################################################
# _summary_init()
#
# Called once from ProjectBootstrap.cmake.
# 1. Prints the project info box immediately (before add_subdirectory calls)
# 2. Defers the configuration summary and dependency graph to end of configure
###############################################################################

function(_summary_init)
    # Project info box (printed now, before modules are added)
    print_section_start("Project Info")
    print_section_line("Name" "${PROJECT_NAME}")
    print_section_line("Version" "${PROJECT_VERSION}")
    print_section_line("Description" "${PROJECT_DESCRIPTION}")
    print_section_line("Source" "${CMAKE_SOURCE_DIR}")
    print_section_line("Build" "${CMAKE_BINARY_DIR}")
    print_section_end()

    # Defer config summary to end of configure
    cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _summary_print_deferred)
endfunction()

###############################################################################
# _summary_print_deferred()
#
# Called at end of configure via cmake_language(DEFER).
# Prints config box and generates dependency graph.
###############################################################################

function(_summary_print_deferred)
    # Dependency graph
    print_box("Module Dependency Graph")
    _summary_generate_dot_graph(_dot_path)
    if(_dot_path)
        message(STATUS "Run: dot -Tpng ${_dot_path} -o dependencies.png")
    endif()

    # Configuration box
    print_section_start("Configuration")
    print_section_line("Compiler" "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    print_section_line("Standard" "C++${CMAKE_CXX_STANDARD}")
    print_section_line("Build Type" "${CMAKE_BUILD_TYPE}")

    # Features from _ENABLED_FEATURES (populated by each module)
    if(_ENABLED_FEATURES)
        list(JOIN _ENABLED_FEATURES ", " _features_str)
        print_section_line("Features" "${_features_str}")
    else()
        print_section_line("Features" "None")
    endif()

    # Compilation flags (line by line)
    print_section_line("Compilation Flags" "")
    set(_cxx_flags "")
    if(CMAKE_CXX_FLAGS)
        list(APPEND _cxx_flags "${CMAKE_CXX_FLAGS}")
    endif()
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _config_suffix)
    set(_config_flags_var "CMAKE_CXX_FLAGS_${_config_suffix}")
    if(${_config_flags_var})
        list(APPEND _cxx_flags "${${_config_flags_var}}")
    endif()
    foreach(_flag IN LISTS _cxx_flags)
        if(_flag)
            print_status_line("" "${_flag}")
        endif()
    endforeach()

    # Sanitizers warning
    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        print_section_line("Warning" "Sanitizers active - slower execution")
    endif()

    print_section_end()
endfunction()

###############################################################################
# _summary_generate_dot_graph()
#
# Generates ${CMAKE_BINARY_DIR}/dependencies.dot (Graphviz).
# Green edges = public, red edges = private.
##############################################################################

function(_summary_generate_dot_graph _dot_path_var)
    # Get the list of module names
    get_property(_module_names GLOBAL PROPERTY PROJECT_MODULE_NAMES)
    if(NOT _module_names)
        set(${_dot_path_var} "" PARENT_SCOPE)
        return()
    endif()

    # Create the dot file
    set(_dot_path "${CMAKE_BINARY_DIR}/dependencies.dot")
    set(_dot_content "digraph dependencies {\n")
    string(APPEND _dot_content "  rankdir=LR;\n")
    string(APPEND _dot_content "  node [shape=box, style=rounded];\n")
    string(APPEND _dot_content "  compound=true;\n\n")

    # Internal targets = our modules
    set(_internal_targets "${_module_names}")

    # Add nodes: internal (blue/cyan) vs external (gray). Version in node label when available.
    foreach(_name IN LISTS _module_names)
        get_property(_type GLOBAL PROPERTY "PROJECT_MODULE_${_name}_TYPE")
        get_property(_ver GLOBAL PROPERTY "PROJECT_MODULE_${_name}_VERSION")
        if(_ver)
            set(_node_label "${_name}\\n${_ver}")
        else()
            set(_node_label "${_name}")
        endif()
        if(_type STREQUAL "executable")
            string(APPEND _dot_content "  \"${_name}\" [label=\"${_node_label}\", fillcolor=lightcyan, style=filled];\n")
        else()
            string(APPEND _dot_content "  \"${_name}\" [label=\"${_node_label}\", fillcolor=lightblue, style=filled];\n")
        endif()
    endforeach()

    # Add edges: public (green), private (red)
    set(_external_seen "")
    foreach(_name IN LISTS _module_names)
        get_property(_pub GLOBAL PROPERTY "PROJECT_MODULE_${_name}_PUBLIC")
        get_property(_priv GLOBAL PROPERTY "PROJECT_MODULE_${_name}_PRIVATE")

        foreach(_dep IN LISTS _pub)
            if(_dep)
                string(APPEND _dot_content "  \"${_name}\" -> \"${_dep}\" [color=darkgreen, fontcolor=darkgreen];\n")
                if(NOT _dep IN_LIST _internal_targets)
                    list(APPEND _external_seen "${_dep}")
                endif()
            endif()
        endforeach()
        foreach(_dep IN LISTS _priv)
            if(_dep)
                string(APPEND _dot_content "  \"${_name}\" -> \"${_dep}\" [color=darkred, fontcolor=darkred];\n")
                if(NOT _dep IN_LIST _internal_targets)
                    list(APPEND _external_seen "${_dep}")
                endif()
            endif()
        endforeach()
    endforeach()

    # External nodes (not in our module list)
    list(REMOVE_DUPLICATES _external_seen)
    foreach(_ext IN LISTS _external_seen)
        if(NOT _ext IN_LIST _internal_targets)
            string(APPEND _dot_content "  \"${_ext}\" [fillcolor=lightgray, style=filled];\n")
        endif()
    endforeach()

    string(APPEND _dot_content "\n")
    string(APPEND _dot_content "}\n")

    file(WRITE "${_dot_path}" "${_dot_content}")
    set(${_dot_path_var} "${_dot_path}" PARENT_SCOPE)
endfunction()
