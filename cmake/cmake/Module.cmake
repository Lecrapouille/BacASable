###############################################################################
# Module Building Functions
#
# High-level functions for creating and installing libraries and executables
# with their associated unit tests, mock libraries and installation.
#
# Public API:
#   add_module_library(<name> ...)      - Create a library module (static/shared)
#   add_module_executable(<name> ...)   - Create an executable module
#
###############################################################################

# Verify that ProjectBootstrap.cmake was included first
if(NOT DEFINED _MODULE_HEADER_EXTENSIONS OR NOT DEFINED _MODULE_SOURCE_EXTENSIONS)
    message(FATAL_ERROR "Module.cmake requires ProjectBootstrap.cmake to be included first")
endif()

###############################################################################
# _module_glob(<out_var> <base> <subdirs> <dir> <exts>)
#
# Recursively globs files matching any extension in <exts> (semicolon-separated
# list) under <base>/<dir>/ when <subdirs> is empty, or under
# <base>/<dir>/<s>/ for each item <s> in <subdirs>.
#
# Examples:
#   _module_glob(_src  "${base}" "${ARG_SUBDIRECTORIES}" src     "${_MODULE_SOURCE_EXTENSIONS}")
#   _module_glob(_hdrs "${base}" "${ARG_SUBDIRECTORIES}" include "${_MODULE_HEADER_EXTENSIONS}")
###############################################################################

function(_module_glob out base subdirs dir extensions)

    set(_result)
    if(NOT subdirs)
        foreach(_ext IN LISTS extensions)
            file(GLOB_RECURSE _tmp CONFIGURE_DEPENDS "${base}/${dir}/*.${_ext}")
            list(APPEND _result ${_tmp})
        endforeach()
    else()
        foreach(_subdir IN LISTS subdirs)
            foreach(_ext IN LISTS extensions)
                file(GLOB_RECURSE _tmp CONFIGURE_DEPENDS "${base}/${dir}/${_subdir}/*.${_ext}")
                list(APPEND _result ${_tmp})
            endforeach()
        endforeach()
    endif()
    set(${out} "${_result}" PARENT_SCOPE)

endfunction()


###############################################################################
# _module_add_build_banner(<target> <text>)
#
# Attaches a PRE_BUILD hook that prints a separator banner with <text>.
# On Makefile generators PRE_BUILD is treated as PRE_LINK (fires after object
# compilation, before the link step). On Ninja and Visual Studio it fires
# before any build step for the target.
# With parallel builds (-jN) banners from concurrent targets will interleave;
# this is expected and unavoidable — CMake has no per-file compilation hook.
###############################################################################

function(_module_add_build_banner target text)

    set(_sep "════════════════════════════════════════════════════════════════════════════════")
    add_custom_command(TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "${_sep}"
        COMMAND ${CMAKE_COMMAND} -E echo "  ${text}"
        COMMAND ${CMAKE_COMMAND} -E echo "${_sep}"
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMENT "" VERBATIM
    )

endfunction()


###############################################################################
# _module_configure_target(<target> [COMPILE_OPTIONS opt1 opt2 ...])
#
# Apply shared build settings to a production target: warnings, sanitizers,
# coverage, MODULE_NAME definition, split DWARF, and optional per-target
# compile options.
###############################################################################

function(_module_configure_target target)

    cmake_parse_arguments(ARG "" "" "COMPILE_OPTIONS" ${ARGN})

    # Apply warnings
    target_set_warnings(${target})

    # Apply sanitizers
    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        target_enable_sanitizers(${target})
    endif()

    # Apply code coverage
    if(ENABLE_COVERAGE)
        target_enable_coverage(${target})
    endif()

    # Apply module name definition
    target_compile_definitions(${target} PRIVATE MODULE_NAME="${target}")

    # Apply split DWARF
    target_enable_split_dwarf(${target})

    # Apply additional compile options
    if(ARG_COMPILE_OPTIONS)
        target_compile_options(${target} PRIVATE ${ARG_COMPILE_OPTIONS})
    endif()

endfunction()


###############################################################################
# _module_configure_test_target(<target>)
#
# Apply test-only settings: source-path definitions (for fixture helpers),
# warnings, sanitizers, coverage, and CTest registration via
# discover_tests_for_target().
###############################################################################

function(_module_configure_test_target target)

    target_compile_definitions(${target} PRIVATE
        CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        CMAKE_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        CMAKE_PROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    )

    target_set_warnings(${target})

    if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
        target_enable_sanitizers(${target})
    endif()

    if(ENABLE_COVERAGE)
        target_enable_coverage(${target})
    endif()

    discover_tests_for_target(${target})

endfunction()


###############################################################################
# _module_install_runtime_deps(<target>)
#
# Deploy shared library dependencies (.so) at install time.
# Excludes system libraries (/lib, /usr/lib).
###############################################################################

function(_module_install_runtime_deps target)

    install(CODE "
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES $<TARGET_FILE:${target}>
            RESOLVED_DEPENDENCIES_VAR   _resolved_deps
            UNRESOLVED_DEPENDENCIES_VAR _unresolved_deps
            PRE_EXCLUDE_REGEXES  \"^/lib\" \"^/usr/lib\"
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


###############################################################################
# _module_add_gtest_target(<name>
#   [LINK_TESTED_TARGET <target>]   Link the library under test (library modules).
#                                   Omit for executable modules (app sources are
#                                   passed via EXTRA_SOURCES instead).
#   [DEPENDENCIES dep1 ...]         Additional link dependencies (or mock targets).
#   [SUBDIRECTORIES s1 ...]         Restrict test source glob to ${TEST_DIR_NAME}/<s>/.
#   [EXTRA_SOURCES src1 ...]        Extra sources compiled into the test binary
#                                   (app sources minus main for executable modules).
# )
#
# Creates <name>${TEST_TARGET_SUFFIX}: discovers ${TEST_DIR_NAME}/, links GTest,
# applies test settings.
###############################################################################

function(_module_add_gtest_target name)

    set(_single_values LINK_TESTED_TARGET)
    set(_multi_values  DEPENDENCIES SUBDIRECTORIES EXTRA_SOURCES)
    cmake_parse_arguments(ARG "" "${_single_values}" "${_multi_values}" ${ARGN})

    set(_base "${CMAKE_CURRENT_SOURCE_DIR}")
    _module_glob(_test_sources "${_base}" "${ARG_SUBDIRECTORIES}" ${TEST_DIR_NAME} "${_MODULE_SOURCE_EXTENSIONS}")

    if(NOT _test_sources)
        summary_register_module_without_tests(${name})
        return()
    endif()

    set(_test_target ${name}${TEST_TARGET_SUFFIX})
    message(STATUS "  Adding test target: ${_test_target}")
    add_executable(${_test_target} ${_test_sources} ${ARG_EXTRA_SOURCES})

    # Optional backward-cpp stacktrace init source
    if(DEFINED STACKTRACE_INIT_SOURCE AND STACKTRACE_INIT_SOURCE)
        target_sources(${_test_target} PRIVATE ${STACKTRACE_INIT_SOURCE})
    endif()

    # Crash stack traces
    if(ENABLE_STACKTRACE AND TARGET Backward::Backward)
        target_link_libraries(${_test_target} PRIVATE Backward::Backward)
    endif()

    target_include_directories(${_test_target} PRIVATE
        ${_base}/include
        ${_base}/src
    )

    # LINK_TESTED_TARGET is empty for executable modules (app sources are in
    # EXTRA_SOURCES), non-empty for library modules. CMake ignores empty items.
    target_link_libraries(${_test_target} PRIVATE
        ${ARG_LINK_TESTED_TARGET}
        ${ARG_DEPENDENCIES}
        GTest::gtest_main
        GTest::gtest
        GTest::gmock
    )

    _module_configure_test_target(${_test_target})

endfunction()


###############################################################################
# library(<TargetName>
#                    [SHARED]
#                    [NO_INSTALL]
#                    [VERSION <version>]
#                    [PCH <path>]
#                    [COMPILE_OPTIONS opt1 opt2 ...]
#                    [PUBLIC_DEPENDENCIES dep1 dep2 ...]
#                    [PRIVATE_DEPENDENCIES dep1 dep2 ...]
#                    [TEST_DEPENDENCIES dep1 dep2 ...]
#                    [SUBDIRECTORIES s1 [s2 ...]]
#                    [SOURCES src1 [src2 ...]])
#
# Create a static or shared library module with automatic source discovery,
# testing, mock library creation and installation.
#
# Source discovery modes (mutually exclusive):
#   - SUBDIRECTORIES: Glob sources in src/<s>/ and headers in include/<s>/
#   - SOURCES: Explicit list of source files (no globbing)
#
# This function:
#   1. Discovers sources in src/ (or src/<s>/ when SUBDIRECTORIES is set) and
#      headers in include/ (or include/<s>/)
#   2. Creates a STATIC (default) or SHARED library target
#   3. Creates an alias: ${PROJECT_NAME}::${TargetName}
#   4. Optionally creates a mock library from mock/ (or mock/<s>/) directory
#   5. Configures installation (component: devel) unless NO_INSTALL
#   6. Creates a unit test target if BUILD_TESTING is ON
#
# Expected directory structure (flat layout; or include/<s>/, src/<s>/, ...):
#   <module>/
#   ├── CMakeLists.txt
#   ├── include/<module>/   # Public headers (or include/<s>/ with SUBDIRECTORIES)
#   ├── src/                # Implementation + private headers
#   ├── mocks/              # Mock implementations (optional)
#   ├── pch/pch.hpp         # Custom PCH (optional)
#   └── ${TEST_DIR_NAME}/   # Unit tests (optional, configurable via TEST_DIR_NAME)
#
# Example (flat layout):
#   library(database
#       VERSION 1.2.0
#       PCH pch/pch.hpp
#       COMPILE_OPTIONS -Wno-conversion
#       PUBLIC_DEPENDENCIES  nlohmann_json::nlohmann_json
#       PRIVATE_DEPENDENCIES SQLite::SQLite3
#       TEST_DEPENDENCIES    testutils
#   )
#
# Example (multi-subdirectory layout):
#   library(kinematics
#       VERSION 0.1.0
#       SUBDIRECTORIES kinematics
#       PCH pch/pch.hpp
#       PUBLIC_DEPENDENCIES mp-units::mp-units
#   )
###############################################################################

function(library TargetName)

    # Parse arguments
    set(_options       NO_INSTALL SHARED)
    set(_single_values PCH VERSION)
    set(_multi_values
        PUBLIC_DEPENDENCIES
        PRIVATE_DEPENDENCIES
        TEST_DEPENDENCIES
        COMPILE_OPTIONS
        SUBDIRECTORIES
        SOURCES
    )
    cmake_parse_arguments(ARG "${_options}" "${_single_values}" "${_multi_values}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_module_library: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    # Validate mutually exclusive options
    if(ARG_SUBDIRECTORIES AND ARG_SOURCES)
        message(FATAL_ERROR "add_module_library(${TargetName}): SUBDIRECTORIES and SOURCES are mutually exclusive")
    endif()

    # Set library type and label
    set(_lib_type  STATIC)
    set(_lib_label "static")
    if(ARG_SHARED)
        set(_lib_type  SHARED)
        set(_lib_label "shared")
    endif()

    # Print module info
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

    # Discover sources and headers
    set(_base "${CMAKE_CURRENT_SOURCE_DIR}")

    if(ARG_SOURCES)
        # Explicit source list provided
        set(_sources ${ARG_SOURCES})
        set(_headers_public)
        set(_headers_private)
        # Glob headers only (sources are explicit)
        _module_glob(_headers_public  "${_base}" "" include "${_MODULE_HEADER_EXTENSIONS}")
        _module_glob(_headers_private "${_base}" "" src     "${_MODULE_HEADER_EXTENSIONS}")
    else()
        # Automatic source discovery via globbing
        _module_glob(_headers_public  "${_base}" "${ARG_SUBDIRECTORIES}" include "${_MODULE_HEADER_EXTENSIONS}")
        _module_glob(_headers_private "${_base}" "${ARG_SUBDIRECTORIES}" src     "${_MODULE_HEADER_EXTENSIONS}")
        _module_glob(_sources         "${_base}" "${ARG_SUBDIRECTORIES}" src     "${_MODULE_SOURCE_EXTENSIONS}")
    endif()

    if(NOT _sources)
        if(ARG_SOURCES)
            message(FATAL_ERROR
                "add_module_library(${TargetName}): SOURCES list is empty")
        elseif(NOT ARG_SUBDIRECTORIES)
            message(FATAL_ERROR
                "add_module_library(${TargetName}): No sources under ${_base}/src/")
        else()
            set(_paths)
            foreach(_subdir IN LISTS ARG_SUBDIRECTORIES)
                list(APPEND _paths "${_base}/src/${_subdir}/")
            endforeach()
            message(FATAL_ERROR
                "add_module_library(${TargetName}): No sources under: ${_paths}")
        endif()
    endif()

    # Create library target
    add_library(${TargetName} ${_lib_type}
        ${_headers_public} ${_headers_private} ${_sources}
    )

    # Set version property (used by install export and SOVERSION)
    if(ARG_VERSION)
        set_target_properties(${TargetName} PROPERTIES
            VERSION   ${ARG_VERSION}
            SOVERSION ${ARG_VERSION}
        )
        message(STATUS "  Version: ${ARG_VERSION}")
    endif()

    # Create mock library (only when BUILD_TESTING is ON)
    set(_all_targets ${TargetName})
    if(BUILD_TESTING AND EXISTS "${_base}/mocks")
        _module_glob(_mock_sources "${_base}" "${ARG_SUBDIRECTORIES}" mocks "${_MODULE_SOURCE_EXTENSIONS}")
        if(_mock_sources)
            set(_mock_target mock_${TargetName})
            add_library(${_mock_target} STATIC ${_headers_public} ${_mock_sources})
            target_include_directories(${_mock_target} PUBLIC
                $<BUILD_INTERFACE:${_base}/mocks>
            )
            target_link_libraries(${_mock_target} PUBLIC GTest::gmock)
            list(APPEND _all_targets ${_mock_target})
            message(STATUS "  Mock library: ${_mock_target}")
        endif()
    endif()

    # Apply shared configuration to both the real target and the mock
    foreach(_target IN LISTS _all_targets)
        target_include_directories(${_target} PUBLIC
            $<BUILD_INTERFACE:${_base}/include>
        )
        if(ARG_PUBLIC_DEPENDENCIES)
            target_link_libraries(${_target} PUBLIC ${ARG_PUBLIC_DEPENDENCIES})
        endif()
        add_library(${PROJECT_NAME}::${_target} ALIAS ${_target})
    endforeach()

    if(ARG_PUBLIC_DEPENDENCIES)
        message(STATUS "  Public dependencies: ${ARG_PUBLIC_DEPENDENCIES}")
    endif()

    # Add build banner
    _module_add_build_banner(${TargetName} "Linked ${_lib_label} library: ${TargetName}")

    # Set shared library runtime path
    if(ARG_SHARED)
        set_target_properties(${TargetName} PROPERTIES
            INSTALL_RPATH            "$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
    endif()

    # Set include directories
    target_include_directories(${TargetName}
        PUBLIC  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE ${_base}/src
    )

    # Link private dependencies
    if(ARG_PRIVATE_DEPENDENCIES)
        message(STATUS "  Private dependencies: ${ARG_PRIVATE_DEPENDENCIES}")
        target_link_libraries(${TargetName} PRIVATE ${ARG_PRIVATE_DEPENDENCIES})
    endif()

    # Configure target: warnings, sanitizers, coverage, module name definition, split DWARF, and optional per-target compile options
    _module_configure_target(${TargetName} COMPILE_OPTIONS ${ARG_COMPILE_OPTIONS})

    # Precompile headers
    if(ARG_PCH)
        target_configure_pch(${TargetName} PCH ${ARG_PCH})
    else()
        target_configure_pch(${TargetName})
    endif()

    # Install library target and dependencies
    if(NOT ARG_NO_INSTALL)
        install(TARGETS ${TargetName}
            EXPORT ${TargetName}Targets
            ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT ${_COMPONENT_DEV}
        )

        # Install debug symbols for shared libraries
        if(ARG_SHARED)
            install_separated_debug_symbols(
                ${TargetName} "${CMAKE_INSTALL_LIBDIR}" ${_COMPONENT_DEV})
        endif()

        # Install include directories
        if(EXISTS "${_base}/include")
            if(ARG_SUBDIRECTORIES)
                foreach(_s IN LISTS ARG_SUBDIRECTORIES)
                    if(EXISTS "${_base}/include/${_s}")
                        install(DIRECTORY "include/${_s}/"
                            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                            COMPONENT ${_COMPONENT_DEV}
                        )
                    endif()
                endforeach()
            else()
                install(DIRECTORY include/
                    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                    COMPONENT ${_COMPONENT_DEV}
                )
            endif()
        endif()

        # Export targets for find_package()
        install(EXPORT ${TargetName}Targets
            FILE ${TargetName}Targets.cmake
            NAMESPACE ${EXPORT_NAMESPACE}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAMESPACE}
            COMPONENT ${_COMPONENT_DEV}
        )
    endif()

    # Create unit test target
    if(BUILD_TESTING)
        _module_add_gtest_target(${TargetName}
            LINK_TESTED_TARGET ${TargetName}
            DEPENDENCIES   ${ARG_TEST_DEPENDENCIES}
            SUBDIRECTORIES ${ARG_SUBDIRECTORIES}
        )
    endif()

endfunction()


###############################################################################
# executable(<TargetName>
#            [NO_INSTALL]
#            [VERSION <version>]
#            [PCH <path>]
#            [COMPILE_OPTIONS opt1 opt2 ...]
#            [DEPENDENCIES dep1 dep2 ...]
#            [TEST_DEPENDENCIES dep1 dep2 ...]
#            [SUBDIRECTORIES s1 [s2 ...]]
#            [SOURCES src1 [src2 ...]])
#
# Create an executable with automatic source discovery and testing.
#
# Source discovery modes (mutually exclusive):
#   - SUBDIRECTORIES: Glob sources in src/<s>/ and headers in include/<s>/
#   - SOURCES: Explicit list of source files (no globbing)
#
# This function:
#   1. Discovers sources in src/ (or src/<s>/ when SUBDIRECTORIES is set)
#      and headers in include/ and src/
#   2. Creates an executable target
#   3. Links Backward::Backward for crash stack traces (if available)
#   4. Configures installation (component: runtime) unless NO_INSTALL
#   5. Deploys runtime shared library dependencies at install time
#   6. Creates a unit test target if BUILD_TESTING is ON
#
# Test dependency resolution:
#   - If TEST_DEPENDENCIES is set, tests link against those (for mock injection)
#   - Otherwise, tests fall back to DEPENDENCIES (real implementations)
#   - App sources (minus main.cpp) are compiled into the test binary so that
#     application logic can be tested without an entry-point conflict.
#
# Example:
#   executable(bar
#       VERSION 2.0.0
#       PCH pch/pch.hpp
#       COMPILE_OPTIONS -Wno-shadow
#       DEPENDENCIES      foo spdlog::spdlog
#       TEST_DEPENDENCIES mock_foo
#   )
###############################################################################

function(executable TargetName)

    # Parse arguments
    set(_options       NO_INSTALL)
    set(_single_values PCH VERSION)
    set(_multi_values  DEPENDENCIES TEST_DEPENDENCIES COMPILE_OPTIONS SUBDIRECTORIES SOURCES)
    cmake_parse_arguments(ARG "${_options}" "${_single_values}" "${_multi_values}" ${ARGN})
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_module_executable: Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    # Validate mutually exclusive options
    if(ARG_SUBDIRECTORIES AND ARG_SOURCES)
        message(FATAL_ERROR "add_module_executable(${TargetName}): SUBDIRECTORIES and SOURCES are mutually exclusive")
    endif()

    # Set private dependencies (includes Backward if present)
    set(_exec_private_deps "${ARG_DEPENDENCIES}")
    if(ENABLE_STACKTRACE AND TARGET Backward::Backward)
        list(APPEND _exec_private_deps Backward::Backward)
    endif()

    # Print module info
    if(ARG_VERSION)
        message(STATUS "Adding executable: ${TargetName} (${ARG_VERSION})")
    else()
        message(STATUS "Adding executable: ${TargetName}")
    endif()

    # Register module with summary
    summary_register_module("${TargetName}" "executable"
        "" "${_exec_private_deps}"
        VERSION "${ARG_VERSION}"
    )

    # Source and header discovery
    set(_base "${CMAKE_CURRENT_SOURCE_DIR}")

    if(ARG_SOURCES)
        # Explicit source list provided
        set(_sources ${ARG_SOURCES})
        # Glob headers only (sources are explicit)
        _module_glob(_headers_include "${_base}" "" include "${_MODULE_HEADER_EXTENSIONS}")
        _module_glob(_headers_src     "${_base}" "" src     "${_MODULE_HEADER_EXTENSIONS}")
    else()
        # Automatic source discovery via globbing
        _module_glob(_headers_include "${_base}" "${ARG_SUBDIRECTORIES}" include "${_MODULE_HEADER_EXTENSIONS}")
        _module_glob(_headers_src     "${_base}" "${ARG_SUBDIRECTORIES}" src     "${_MODULE_HEADER_EXTENSIONS}")
        _module_glob(_sources         "${_base}" "${ARG_SUBDIRECTORIES}" src     "${_MODULE_SOURCE_EXTENSIONS}")
    endif()
    set(_headers ${_headers_include} ${_headers_src})

    if(NOT _sources)
        if(ARG_SOURCES)
            message(FATAL_ERROR
                "add_module_executable(${TargetName}): SOURCES list is empty")
        elseif(NOT ARG_SUBDIRECTORIES)
            message(FATAL_ERROR
                "add_module_executable(${TargetName}): No sources under ${_base}/src/")
        else()
            set(_paths)
            foreach(_subdir IN LISTS ARG_SUBDIRECTORIES)
                list(APPEND _paths "${_base}/src/${_subdir}/")
            endforeach()
            message(FATAL_ERROR
                "add_module_executable(${TargetName}): No sources under: ${_paths}")
        endif()
    endif()

    # Create executable target
    add_executable(${TargetName} ${_headers} ${_sources})

    # Optional backward-cpp stacktrace init source
    if(DEFINED STACKTRACE_INIT_SOURCE AND STACKTRACE_INIT_SOURCE)
        target_sources(${TargetName} PRIVATE ${STACKTRACE_INIT_SOURCE})
    endif()

    # Add build banner
    _module_add_build_banner(${TargetName} "Linked executable: ${TargetName}")

    # Set version
    if(ARG_VERSION)
        message(STATUS "  Version: ${ARG_VERSION}")
    endif()

    # Set include directories
    target_include_directories(${TargetName} PRIVATE
        ${_base}/include
        ${_base}/src
    )

    # Link private dependencies
    if(ARG_DEPENDENCIES)
        message(STATUS "  Dependencies: ${ARG_DEPENDENCIES}")
        target_link_libraries(${TargetName} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    # Link backward-cpp for crash stack traces
    if(ENABLE_STACKTRACE AND TARGET Backward::Backward)
        target_link_libraries(${TargetName} PRIVATE Backward::Backward)
    endif()

    # Configure target: warnings, sanitizers, coverage, module name definition,
    # split DWARF, and optional per-target compile options
    _module_configure_target(${TargetName} COMPILE_OPTIONS ${ARG_COMPILE_OPTIONS})

    # Precompile headers
    if(ARG_PCH)
        target_configure_pch(${TargetName} PCH ${ARG_PCH})
    else()
        target_configure_pch(${TargetName})
    endif()

    # Install executable target and dependencies
    if(NOT ARG_NO_INSTALL)
        # Install executable target
        install(TARGETS ${TargetName}
            EXPORT ${TargetName}Targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT ${_COMPONENT_RUNTIME}
        )

        # Install debug symbols for executables
        install_separated_debug_symbols(
            ${TargetName} "${CMAKE_INSTALL_BINDIR}" ${_COMPONENT_RUNTIME})

        # Deploy runtime shared library dependencies at install time
        _module_install_runtime_deps(${TargetName})
        message(STATUS "  Runtime dependency deployment: enabled")

        # Export targets for find_package()
        install(EXPORT ${TargetName}Targets
            FILE ${TargetName}Targets.cmake
            NAMESPACE ${EXPORT_NAMESPACE}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAMESPACE}
            COMPONENT ${_COMPONENT_DEV}
        )
    endif()

    # Create unit test target
    # TEST_DEPENDENCIES overrides DEPENDENCIES for mock injection;
    # falls back to DEPENDENCIES when not specified.
    if(BUILD_TESTING)
        if(ARG_TEST_DEPENDENCIES)
            set(_test_dependencies ${ARG_TEST_DEPENDENCIES})
        else()
            set(_test_dependencies ${ARG_DEPENDENCIES})
        endif()

        # Discover app sources so tests can link application logic.
        # main.cpp is excluded: it defines main() which would conflict with
        # GTest::gtest_main's entry point.
        if(ARG_SOURCES)
            set(_app_sources ${ARG_SOURCES})
        else()
            _module_glob(_app_sources "${_base}" "${ARG_SUBDIRECTORIES}" src "${_MODULE_SOURCE_EXTENSIONS}")
        endif()
        list(FILTER _app_sources EXCLUDE REGEX ".*/main\\.cpp$")
        _module_add_gtest_target(${TargetName}
            DEPENDENCIES   ${_test_dependencies}
            SUBDIRECTORIES ${ARG_SUBDIRECTORIES}
            EXTRA_SOURCES  ${_app_sources}
        )
    endif()

endfunction()
