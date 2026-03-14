###############################################################################
# Internal Helpers for building modules
#
# Private helper functions used by ModuleAPI.cmake. Not intended for
# direct use in module CMakeLists.txt files.
###############################################################################


###############################################################################
# internal_add_build_banner(<TargetName> <BannerText>)
#
# Adds a custom command to the target to print a banner with the given text.
###############################################################################

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


###############################################################################
# internal_configure_target(<TargetName> [COMPILE_OPTIONS opt1 opt2 ...])
#
# Apply shared build settings: warnings, sanitizers, coverage, MODULE_NAME,
# debug symbols, and optional per-target compile options.
###############################################################################

function(internal_configure_target TargetName)
    set(_multi_values COMPILE_OPTIONS)
    cmake_parse_arguments(ARG "" "" "${_multi_values}" ${ARGN})

    # Apply compiler warnings
    target_set_warnings(${TargetName})

    # Apply sanitizers
    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        target_enable_sanitizers(${TargetName})
    endif()

    # Apply code coverage
    if(ENABLE_COVERAGE)
        target_enable_coverage(${TargetName})
    endif()

    # Apply module name definition
    target_compile_definitions(${TargetName} PRIVATE MODULE_NAME="${TargetName}")

    # Apply split DWARF
    target_enable_split_dwarf(${TargetName})

    # Apply optional per-target compile options
    if(ARG_COMPILE_OPTIONS)
        target_compile_options(${TargetName} PRIVATE ${ARG_COMPILE_OPTIONS})
    endif()
endfunction()


###############################################################################
# internal_configure_test_target(<TestTargetName>)
###############################################################################

function(internal_configure_test_target TestTargetName)
    target_compile_definitions(${TestTargetName} PRIVATE
        CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        CMAKE_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        CMAKE_PROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    )

    # Apply code coverage
    if(ENABLE_COVERAGE)
        target_enable_coverage(${TestTargetName})
    endif()

    # Apply sanitizers
    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        target_enable_sanitizers(${TestTargetName})
    endif()

    # Discover tests
    discover_tests_for_target(${TestTargetName})
endfunction()


###############################################################################
# internal_add_module_test(<TargetName> <TestedTarget>
#                          [DEPENDENCIES dep1 dep2 ...])
#
# Create unit tests for a library module.
###############################################################################
function(internal_add_module_test TargetName TestedTarget)
    # Parse arguments
    set(_multi_values DEPENDENCIES)
    cmake_parse_arguments(ARG "" "" "${_multi_values}" ${ARGN})
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "internal_add_module_test: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    # Create test target name
    set(_test_target ${TargetName}_tests)

    # Find test sources and create test target
    file(GLOB_RECURSE _test_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc"
    )
    if(NOT _test_sources)
        message(WARNING "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}/tests/")
        return()
    endif()

    add_executable(${_test_target} ${_test_sources})

    # Apply stacktrace initialization
    if(DEFINED STACKTRACE_INIT_SOURCE AND STACKTRACE_INIT_SOURCE)
        target_sources(${_test_target} PRIVATE ${STACKTRACE_INIT_SOURCE})
    endif()
    
    # Link backward-cpp if available
    if(TARGET Backward::Backward)
        target_link_libraries(${_test_target} PRIVATE Backward::Backward)
    endif()

    # Link with GoogleTest
    target_link_libraries(${_test_target} PRIVATE
        ${TestedTarget} ${ARG_DEPENDENCIES} GTest::gtest_main GTest::gtest GTest::gmock
    )

    # Include directories
    target_include_directories(${_test_target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    # Configure test target
    internal_configure_test_target(${_test_target})
endfunction()


###############################################################################
# internal_add_executable_test(<TargetName>
#                              [DEPENDENCIES dep1 dep2 ...])
#
# Create unit tests for an executable module.
###############################################################################

function(internal_add_executable_test TargetName)
    # Parse arguments
    set(_multi_values DEPENDENCIES)
    cmake_parse_arguments(ARG "" "" "${_multi_values}" ${ARGN})
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "internal_add_executable_test: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    # Create test target name
    set(_test_target ${TargetName}_tests)

    # Find test sources
    file(GLOB_RECURSE _test_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc"
    )

    if(NOT _test_sources)
        message(WARNING "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}/tests/")
        return()
    endif()

    # Find application sources
    file(GLOB_RECURSE _app_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    )
    list(FILTER _app_sources EXCLUDE REGEX ".*main\\.c.*$")

    # Create test target
    message(STATUS "  Adding test target: ${_test_target}")
    add_executable(${_test_target} ${_test_sources} ${_app_sources})

    # Apply stacktrace initialization
    if(DEFINED STACKTRACE_INIT_SOURCE AND STACKTRACE_INIT_SOURCE)
        target_sources(${_test_target} PRIVATE ${STACKTRACE_INIT_SOURCE})
    endif()

    # Include directories
    target_include_directories(${_test_target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Link backward-cpp if available
    if(TARGET Backward::Backward)
        target_link_libraries(${_test_target} PRIVATE Backward::Backward)
    endif()

    # Link with GoogleTest
    target_link_libraries(${_test_target} PRIVATE
        ${ARG_DEPENDENCIES} GTest::gtest_main GTest::gtest GTest::gmock
    )

    # Configure test target
    internal_configure_test_target(${_test_target})
endfunction()


###############################################################################
# internal_install_runtime_dependencies(<TargetName>)
#
# Deploy shared library dependencies (.so) at install time.
# Excludes system libraries (/lib, /usr/lib).
###############################################################################
function(internal_install_runtime_dependencies TargetName)
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
