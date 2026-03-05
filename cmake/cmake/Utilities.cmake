# ============================================================================
# Module Building Utilities
# ============================================================================
#
# High-level functions for creating libraries, executables, and their
# associated unit tests with consistent project structure and minimal
# boilerplate.
#
# Public API:
#   add_module_library(<name> ...)      - Create a library module (static/shared)
#   add_module_executable(<name> ...)   - Create an executable module
#   install_module_dependencies(<name>) - Deploy shared library dependencies
#
# ============================================================================

include(CompilerWarnings)

# ############################################################################
#                         INTERNAL HELPER FUNCTIONS
# ############################################################################


# ----------------------------------------------------------------------------
# internal_add_build_banner(<TargetName> <BannerText>)
# ----------------------------------------------------------------------------
# Visual separator displayed at build time before compiling a target.
#
function(internal_add_build_banner TargetName BannerText)
    set(_sep "════════════════════════════════════════════════════════════════════════════════")
    add_custom_command(TARGET ${TargetName} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "${_sep}"
        COMMAND ${CMAKE_COMMAND} -E echo "  ${BannerText}"
        COMMAND ${CMAKE_COMMAND} -E echo "${_sep}"
        COMMENT ""
        VERBATIM
    )
endfunction()


# ----------------------------------------------------------------------------
# internal_configure_target(<TargetName>)
# ----------------------------------------------------------------------------
# Apply shared build settings to a production target: warnings, sanitizers,
# coverage, MODULE_NAME definition, and debug symbols.
#
function(internal_configure_target TargetName)
    target_set_warnings(${TargetName})

    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        target_enable_sanitizers(${TargetName})
    endif()

    if(ENABLE_COVERAGE)
        target_enable_coverage(${TargetName})
    endif()

    target_compile_definitions(${TargetName} PRIVATE MODULE_NAME="${TargetName}")

    target_enable_split_dwarf(${TargetName})
    target_extract_debug_symbols(${TargetName})
endfunction()


# ----------------------------------------------------------------------------
# internal_configure_test_target(<TestTargetName>)
# ----------------------------------------------------------------------------
# Apply test-specific settings: resource path definitions, coverage
# instrumentation, sanitizers, and CTest registration via gtest_discover.
#
function(internal_configure_test_target TestTargetName)
    target_compile_definitions(${TestTargetName} PRIVATE
        CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        CMAKE_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        CMAKE_PROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    )

    if(ENABLE_COVERAGE)
        target_enable_coverage(${TestTargetName})
    endif()

    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        target_enable_sanitizers(${TestTargetName})
    endif()

    include(GoogleTest)
    gtest_discover_tests(${TestTargetName}
        PROPERTIES
            TIMEOUT 120
            LABELS "unit"
    )
endfunction()


# ----------------------------------------------------------------------------
# internal_add_module_test(<TargetName> <TestedTarget>
#                          [DEPENDENCIES dep1 dep2 ...])
# ----------------------------------------------------------------------------
# Create unit tests for a library module.
#
# Globs *.cpp in tests/, links against the tested library and GTest,
# and grants access to private src/ headers for white-box testing.
#
function(internal_add_module_test TargetName TestedTarget)
    set(_multi_values DEPENDENCIES)
    cmake_parse_arguments(ARG "" "" "${_multi_values}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "internal_add_module_test: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(_test_target ${TargetName}_tests)
    file(GLOB_RECURSE _test_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    if(NOT _test_sources)
        message(WARNING "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}/tests/")
        return()
    endif()

    message(STATUS "  Adding test target: ${_test_target}")
    add_executable(${_test_target} ${_test_sources})

    target_link_libraries(${_test_target} PRIVATE
        ${TestedTarget} ${ARG_DEPENDENCIES} GTest::gtest_main
    )

    target_include_directories(${_test_target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    internal_configure_test_target(${_test_target})
endfunction()


# ----------------------------------------------------------------------------
# internal_add_executable_test(<TargetName>
#                              [DEPENDENCIES dep1 dep2 ...])
# ----------------------------------------------------------------------------
# Create unit tests for an executable module.
#
# Recompiles all src/*.cpp except main.cpp together with tests/*.cpp,
# allowing the test to exercise application logic without the entry point.
#
function(internal_add_executable_test TargetName)
    set(_multi_values DEPENDENCIES)
    cmake_parse_arguments(ARG "" "" "${_multi_values}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "internal_add_executable_test: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(_test_target ${TargetName}_tests)
    file(GLOB_RECURSE _test_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    if(NOT _test_sources)
        message(WARNING "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}/tests/")
        return()
    endif()

    file(GLOB_RECURSE _app_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    )
    list(FILTER _app_sources EXCLUDE REGEX ".*main\\.cpp$")

    message(STATUS "  Adding test target: ${_test_target}")
    add_executable(${_test_target} ${_test_sources} ${_app_sources})

    target_include_directories(${_test_target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(${_test_target} PRIVATE
        ${ARG_DEPENDENCIES} GTest::gtest_main
    )

    internal_configure_test_target(${_test_target})
endfunction()


# ############################################################################
#                         PUBLIC API FUNCTIONS
# ############################################################################


# ----------------------------------------------------------------------------
# add_module_library(<TargetName>
#                    [SHARED]
#                    [NO_INSTALL]
#                    [PCH <path>]
#                    [PUBLIC_DEPENDENCIES dep1 dep2 ...]
#                    [PRIVATE_DEPENDENCIES dep1 dep2 ...]
#                    [TEST_DEPENDENCIES dep1 dep2 ...])
# ----------------------------------------------------------------------------
# Create a library module with automatic source discovery and testing.
#
# This function:
#   1. Discovers sources in src/ and headers in include/
#   2. Creates a STATIC (default) or SHARED library target
#   3. Creates an alias: ${PROJECT_NAME}::${TargetName}
#   4. Optionally creates a mock library from mock/ directory
#   5. Configures installation (component: devel)
#   6. Creates a unit test target if BUILD_TESTING is ON
#
# Expected directory structure:
#   <module>/
#   ├── CMakeLists.txt
#   ├── include/<module>/   # Public headers
#   ├── src/                # Implementation + private headers
#   ├── mock/               # Mock implementations (optional)
#   ├── pch/pch.hpp         # Custom PCH (optional)
#   └── tests/              # Unit tests
#
# Example:
#   add_module_library(database
#       PCH pch/pch.hpp
#       PUBLIC_DEPENDENCIES  nlohmann_json::nlohmann_json
#       PRIVATE_DEPENDENCIES SQLite::SQLite3
#       TEST_DEPENDENCIES    testutils
#   )
# ----------------------------------------------------------------------------

function(add_module_library TargetName)
    set(_options NO_INSTALL SHARED)
    set(_single_values PCH)
    set(_multi_values PUBLIC_DEPENDENCIES PRIVATE_DEPENDENCIES TEST_DEPENDENCIES)
    cmake_parse_arguments(ARG "${_options}" "${_single_values}" "${_multi_values}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_module_library: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(ARG_SHARED)
        set(_lib_type SHARED)
        set(_lib_label "shared")
    else()
        set(_lib_type STATIC)
        set(_lib_label "static")
    endif()
    message(STATUS "Adding ${_lib_label} library: ${TargetName}")

    # Source discovery
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
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    if(NOT _sources)
        message(FATAL_ERROR "add_module_library(${TargetName}): No sources in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
    endif()

    # Headers listed for IDE project generation (shows in file tree)
    add_library(${TargetName} ${_lib_type}
        ${_headers_public} ${_headers_private} ${_sources}
    )

    set(_all_targets ${TargetName})

    # Mock library: stub implementations delegating to GMock objects.
    # Created automatically when a mock/ directory exists.
    if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mock")
        file(GLOB_RECURSE _mock_sources CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/mock/*.cpp"
        )
        if(_mock_sources)
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
        add_library(${PROJECT_NAME}::${_target} ALIAS ${_target})
    endforeach()

    if(ARG_PUBLIC_DEPENDENCIES)
        message(STATUS "  Public dependencies: ${ARG_PUBLIC_DEPENDENCIES}")
    endif()

    # Real target only
    internal_add_build_banner(${TargetName} "Building library: ${TargetName} (${_lib_label})")

    if(ARG_SHARED)
        set_target_properties(${TargetName} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
    endif()

    target_include_directories(${TargetName}
        PUBLIC  $<INSTALL_INTERFACE:include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    if(ARG_PRIVATE_DEPENDENCIES)
        message(STATUS "  Private dependencies: ${ARG_PRIVATE_DEPENDENCIES}")
        target_link_libraries(${TargetName} PRIVATE ${ARG_PRIVATE_DEPENDENCIES})
    endif()

    internal_configure_target(${TargetName})

    if(ARG_PCH)
        target_configure_pch(${TargetName} PCH ${ARG_PCH})
    else()
        target_configure_pch(${TargetName})
    endif()

    # Installation (only for top-level project)
    if(PROJECT_IS_TOP_LEVEL AND NOT ARG_NO_INSTALL)
        include(GNUInstallDirs)
        install(TARGETS ${TargetName}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT devel
        )
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
            install(DIRECTORY include/
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                COMPONENT devel
            )
        endif()
    endif()

    if(BUILD_TESTING)
        internal_add_module_test(${TargetName} ${TargetName}
            DEPENDENCIES ${ARG_TEST_DEPENDENCIES}
        )
    endif()

endfunction()


# ----------------------------------------------------------------------------
# add_module_executable(<TargetName>
#                       [NO_INSTALL]
#                       [PCH <path>]
#                       [DEPENDENCIES dep1 dep2 ...]
#                       [TEST_DEPENDENCIES dep1 dep2 ...])
# ----------------------------------------------------------------------------
# Create an executable module with automatic source discovery and testing.
#
# This function:
#   1. Discovers sources in src/ and headers in include/ and src/
#   2. Creates an executable target
#   3. Configures installation (component: runtime)
#   4. Creates a unit test target if BUILD_TESTING is ON
#
# Test dependency resolution:
#   - If TEST_DEPENDENCIES is set, tests link against those (for mock injection)
#   - Otherwise, tests fall back to DEPENDENCIES (real implementations)
#
# Expected directory structure:
#   <module>/
#   ├── CMakeLists.txt
#   ├── include/<module>/   # Application headers
#   ├── src/
#   │   ├── main.cpp        # Entry point (excluded from tests)
#   │   └── App.cpp         # Application logic
#   ├── pch/pch.hpp         # Custom PCH (optional)
#   └── tests/              # Unit tests
#
# Example:
#   add_module_executable(bar
#       PCH pch/pch.hpp
#       DEPENDENCIES      foo spdlog::spdlog
#       TEST_DEPENDENCIES foo_mock
#   )
# ----------------------------------------------------------------------------

function(add_module_executable TargetName)
    set(_options NO_INSTALL)
    set(_single_values PCH)
    set(_multi_values DEPENDENCIES TEST_DEPENDENCIES)
    cmake_parse_arguments(ARG "${_options}" "${_single_values}" "${_multi_values}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_module_executable: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    message(STATUS "Adding executable: ${TargetName}")

    # Source discovery
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

    if(NOT _sources)
        message(FATAL_ERROR "add_module_executable(${TargetName}): No sources in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
    endif()

    add_executable(${TargetName} ${_headers} ${_sources})
    internal_add_build_banner(${TargetName} "Building executable: ${TargetName}")

    target_include_directories(${TargetName} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    if(ARG_DEPENDENCIES)
        message(STATUS "  Dependencies: ${ARG_DEPENDENCIES}")
        target_link_libraries(${TargetName} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    internal_configure_target(${TargetName})

    if(ARG_PCH)
        target_configure_pch(${TargetName} PCH ${ARG_PCH})
    else()
        target_configure_pch(${TargetName})
    endif()

    # Installation (only for top-level project)
    if(PROJECT_IS_TOP_LEVEL AND NOT ARG_NO_INSTALL)
        include(GNUInstallDirs)
        install(TARGETS ${TargetName}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT runtime
        )
    endif()

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


# ============================================================================
# install_module_dependencies(<TargetName>)
# ============================================================================
# Deploy shared library dependencies (.so files) of an executable at install
# time. Excludes system libraries (/lib, /usr/lib).
#
# Example:
#   add_module_executable(myapp DEPENDENCIES somelib)
#   install_module_dependencies(myapp)
# ============================================================================

function(install_module_dependencies TargetName)
    get_target_property(_target_type ${TargetName} TYPE)
    if(NOT _target_type STREQUAL "EXECUTABLE")
        message(FATAL_ERROR "install_module_dependencies: '${TargetName}' must be an EXECUTABLE (got: ${_target_type})")
    endif()

    message(STATUS "  Adding runtime dependency deployment for: ${TargetName}")

    install(CODE "
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES $<TARGET_FILE:${TargetName}>
            RESOLVED_DEPENDENCIES_VAR _resolved_deps
            UNRESOLVED_DEPENDENCIES_VAR _unresolved_deps
            PRE_EXCLUDE_REGEXES \"^/lib\" \"^/usr/lib\"
            POST_EXCLUDE_REGEXES \"^/lib\" \"^/usr/lib\"
        )
        foreach(_dep \${_resolved_deps})
            message(STATUS \"Installing runtime dependency: \${_dep}\")
            file(INSTALL \${_dep} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib)
        endforeach()
        if(_unresolved_deps)
            message(WARNING \"Unresolved dependencies: \${_unresolved_deps}\")
        endif()
    ")

endfunction()
