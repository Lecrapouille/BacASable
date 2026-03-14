###############################################################################
# Public Module Building Functions
#
# High-level functions for creating and installing libraries and executables
# with their associated unit tests, mock libraries and installation.
#
# Public API:
#   add_module_library(<name> ...)      - Create a library module (static/shared)
#   add_module_executable(<name> ...)   - Create an executable module
###############################################################################

include(ModuleInternal)

###############################################################################
# add_module_library(<TargetName>
#                    [SHARED]
#                    [NO_INSTALL]
#                    [VERSION <version>]
#                    [PCH <path>]
#                    [COMPILE_OPTIONS opt1 opt2 ...]
#                    [PUBLIC_DEPENDENCIES dep1 dep2 ...]
#                    [PRIVATE_DEPENDENCIES dep1 dep2 ...]
#                    [TEST_DEPENDENCIES dep1 dep2 ...])
#
# Create a static or shared library module with automatic source discovery,
# testing, mock library creation and installation.
#
# This function:
#   1. Discovers sources in src/ and headers in include/
#   2. Creates a STATIC (default) or SHARED library target
#   3. Creates an alias: ${PROJECT_NAME}::${TargetName}
#   4. Optionally creates a mock library from mock/ directory
#   5. Configures installation (component: devel) unless NO_INSTALL
#   6. Creates a unit test target if BUILD_TESTING is ON
#   7. Deploys runtime shared library dependencies at install time
#
# Expected directory structure:
#   <module>/
#   ├── CMakeLists.txt
#   ├── include/<module>/   # Public headers
#   ├── src/                # Implementation + private headers
#   ├── mock/               # Mock implementations (optional)
#   ├── pch/pch.hpp         # Custom PCH (optional)
#   └── tests/              # Unit tests (optional)
#
# Example:
#   add_module_library(database
#       VERSION 1.2.0
#       PCH pch/pch.hpp
#       COMPILE_OPTIONS -Wno-conversion
#       PUBLIC_DEPENDENCIES  nlohmann_json::nlohmann_json
#       PRIVATE_DEPENDENCIES SQLite::SQLite3
#       TEST_DEPENDENCIES    testutils
#   )
###############################################################################

function(add_module_library TargetName)
    # Parse arguments
    set(_options NO_INSTALL SHARED)
    set(_single_values PCH VERSION)
    set(_multi_values PUBLIC_DEPENDENCIES PRIVATE_DEPENDENCIES TEST_DEPENDENCIES COMPILE_OPTIONS)
    cmake_parse_arguments(ARG "${_options}" "${_single_values}" "${_multi_values}" ${ARGN})
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_module_library: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    # Determine library type
    if(ARG_SHARED)
        set(_lib_type SHARED)
        set(_lib_label "shared")
    else()
        set(_lib_type STATIC)
        set(_lib_label "static")
    endif()

    # Register module with summary
    if(ARG_VERSION)
        message(STATUS "Adding ${_lib_label} library: ${TargetName} (${ARG_VERSION})")
    else()
        message(STATUS "Adding ${_lib_label} library: ${TargetName}")
    endif()
    summary_register_module("${TargetName}" "${_lib_label}"
        "${ARG_PUBLIC_DEPENDENCIES}"
        "${ARG_PRIVATE_DEPENDENCIES}"
        VERSION "${ARG_VERSION}"
    )

    # Source and header discovery
    file(GLOB_RECURSE _headers_public CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    )
    file(GLOB_RECURSE _headers_private CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    )
    file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    if(NOT _sources)
        message(FATAL_ERROR "add_module_library(${TargetName}): No sources in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
    endif()

    # Create library target
    add_library(${TargetName} ${_lib_type}
        ${_headers_public} ${_headers_private} ${_sources}
    )

    # Version property (used by install export and SOVERSION)
    if(ARG_VERSION)
        set_target_properties(${TargetName} PROPERTIES
            VERSION ${ARG_VERSION}
            SOVERSION ${ARG_VERSION}
        )
        message(STATUS "  Version: ${ARG_VERSION}")
    endif()

    set(_all_targets ${TargetName})

    # Create mock library target if BUILD_TESTING is ON and mock/ directory exists
    if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mock")
        file(GLOB_RECURSE _mock_sources CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/mock/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/mock/*.cc"
        )
        if(_mock_sources)
            # Create mock library target
            set(_mock_target ${TargetName}_mock)
            add_library(${_mock_target} STATIC ${_headers_public} ${_mock_sources})
            target_include_directories(${_mock_target} PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mock>
            )
            target_link_libraries(${_mock_target} PUBLIC GTest::gmock)
            list(APPEND _all_targets ${_mock_target})
            message(STATUS "  Mock library: ${_mock_target}")
        endif()
    endif()

    # Shared configuration (real + mock targets)
    foreach(_target IN LISTS _all_targets)
        target_include_directories(${_target} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )
        if(ARG_PUBLIC_DEPENDENCIES)
            target_link_libraries(${_target} PUBLIC ${ARG_PUBLIC_DEPENDENCIES})
        endif()
        # Create alias for the target
        add_library(${PROJECT_NAME}::${_target} ALIAS ${_target})
    endforeach()

    if(ARG_PUBLIC_DEPENDENCIES)
        message(STATUS "  Public dependencies: ${ARG_PUBLIC_DEPENDENCIES}")
    endif()

    # Real target only
    internal_add_build_banner(${TargetName} "Building library: ${TargetName} (${_lib_label})")

    # Configure shared library properties
    if(ARG_SHARED)
        set_target_properties(${TargetName} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
    endif()

    # Include directories
    target_include_directories(${TargetName}
        PUBLIC  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Link private dependencies
    if(ARG_PRIVATE_DEPENDENCIES)
        message(STATUS "  Private dependencies: ${ARG_PRIVATE_DEPENDENCIES}")
        target_link_libraries(${TargetName} PRIVATE ${ARG_PRIVATE_DEPENDENCIES})
    endif()

    # Configure target
    internal_configure_target(${TargetName} COMPILE_OPTIONS ${ARG_COMPILE_OPTIONS})

    # Configure PCH
    if(ARG_PCH)
        target_configure_pch(${TargetName} PCH ${ARG_PCH})
    else()
        target_configure_pch(${TargetName})
    endif()

    # Installation (only for top-level project, unless NO_INSTALL)
    if(PROJECT_IS_TOP_LEVEL AND NOT ARG_NO_INSTALL)
        include(GNUInstallDirs)
        install(TARGETS ${TargetName}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT devel
        )
        # Install debug symbols for shared libraries
        if(ARG_SHARED)
            install_separated_debug_symbols(${TargetName} "${CMAKE_INSTALL_LIBDIR}" devel)
        endif()
        # Install headers
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
            install(DIRECTORY include/
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                COMPONENT devel
            )
        endif()
    endif()

    # Create unit test target if BUILD_TESTING is ON
    if(BUILD_TESTING)
        internal_add_module_test(${TargetName} ${TargetName}
            DEPENDENCIES ${ARG_TEST_DEPENDENCIES}
        )
    endif()

endfunction()

###############################################################################
# add_module_executable(<TargetName>
#                       [NO_INSTALL]
#                       [VERSION <version>]
#                       [PCH <path>]
#                       [COMPILE_OPTIONS opt1 opt2 ...]
#                       [DEPENDENCIES dep1 dep2 ...]
#                       [TEST_DEPENDENCIES dep1 dep2 ...])
#
# Create an executable module with automatic source discovery and testing.
#
# This function:
#   1. Discovers sources in src/ and headers in include/ and src/
#   2. Creates an executable target
#   3. Links Backward::Backward for crash stack traces (if available)
#   4. Configures installation (component: runtime) unless NO_INSTALL
#   5. Deploys runtime shared library dependencies at install time
#   6. Creates a unit test target if BUILD_TESTING is ON
#   7. Deploys runtime shared library dependencies at install time
#
# Test dependency resolution:
#   - If TEST_DEPENDENCIES is set, tests link against those (for mock injection)
#   - Otherwise, tests fall back to DEPENDENCIES (real implementations)
#
# Example:
#   add_module_executable(bar
#       VERSION 2.0.0
#       PCH pch/pch.hpp
#       COMPILE_OPTIONS -Wno-shadow
#       DEPENDENCIES      foo spdlog::spdlog
#       TEST_DEPENDENCIES foo_mock
#   )
###############################################################################

function(add_module_executable TargetName)
    # Parse arguments
    set(_options NO_INSTALL)
    set(_single_values PCH VERSION)
    set(_multi_values DEPENDENCIES TEST_DEPENDENCIES COMPILE_OPTIONS)
    cmake_parse_arguments(ARG "${_options}" "${_single_values}" "${_multi_values}" ${ARGN})
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_module_executable: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    # Register module with summary
    set(_exec_private_deps "${ARG_DEPENDENCIES}")
    if(TARGET Backward::Backward)
        list(APPEND _exec_private_deps Backward::Backward)
    endif()
    if(ARG_VERSION)
        message(STATUS "Adding executable: ${TargetName} (${ARG_VERSION})")
    else()
        message(STATUS "Adding executable: ${TargetName}")
    endif()
    summary_register_module("${TargetName}" "executable" "" "${_exec_private_deps}"
        VERSION "${ARG_VERSION}"
    )

    # Source and header discovery
    file(GLOB_RECURSE _headers CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    )
    file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    # Check if sources were found
    if(NOT _sources)
        message(FATAL_ERROR "add_module_executable(${TargetName}): No sources in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
    endif()

    # Create executable target
    add_executable(${TargetName} ${_headers} ${_sources})

    # Apply stacktrace initialization
    if(DEFINED STACKTRACE_INIT_SOURCE AND STACKTRACE_INIT_SOURCE)
        target_sources(${TargetName} PRIVATE ${STACKTRACE_INIT_SOURCE})
    endif()
    internal_add_build_banner(${TargetName} "Building executable: ${TargetName}")

    # Version property (used by install export and SOVERSION)
    if(ARG_VERSION)
        message(STATUS "  Version: ${ARG_VERSION}")
    endif()

    # Include directories
    target_include_directories(${TargetName} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Link private dependencies
    if(ARG_DEPENDENCIES)
        message(STATUS "  Dependencies: ${ARG_DEPENDENCIES}")
        target_link_libraries(${TargetName} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    # Auto-link backward-cpp for crash stack traces on executables
    if(TARGET Backward::Backward)
        target_link_libraries(${TargetName} PRIVATE Backward::Backward)
    endif()

    # Configure target
    internal_configure_target(${TargetName} COMPILE_OPTIONS ${ARG_COMPILE_OPTIONS})

    # Configure PCH
    if(ARG_PCH)
        target_configure_pch(${TargetName} PCH ${ARG_PCH})
    else()
        target_configure_pch(${TargetName})
    endif()

    # Installation (only for top-level project, unless NO_INSTALL)
    if(PROJECT_IS_TOP_LEVEL AND NOT ARG_NO_INSTALL)
        include(GNUInstallDirs)
        install(TARGETS ${TargetName}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT runtime
        )
        # Install debug symbols for executables
        install_separated_debug_symbols(${TargetName} "${CMAKE_INSTALL_BINDIR}" runtime)
        # Deploy runtime shared library dependencies
        internal_install_runtime_dependencies(${TargetName})
        message(STATUS "  Runtime dependency deployment: enabled")
    endif()

    # Create unit test target if BUILD_TESTING is ON
    # TEST_DEPENDENCIES overrides DEPENDENCIES for mock injection;
    # falls back to DEPENDENCIES when not specified.
    if(BUILD_TESTING)
        if(ARG_TEST_DEPENDENCIES)
            set(_test_deps ${ARG_TEST_DEPENDENCIES})
        else()
            set(_test_deps ${ARG_DEPENDENCIES})
        endif()
        internal_add_executable_test(${TargetName}
            DEPENDENCIES ${_test_deps}
        )
    endif()

endfunction()
