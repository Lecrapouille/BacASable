###############################################################################
# Code Coverage Configuration
#
# This module provides functions to enable code coverage instrumentation and
# generate coverage reports using gcov/lcov. It creates custom targets for
# coverage: coverage, coverage-open, and coverage-clean.
#
# Note: Coverage should be used with Debug builds for accurate line mapping.
#
# Prerequisites:
#   - GCC or Clang compiler
#   - gcov (usually included with GCC)
#   - lcov (for HTML reports): sudo apt install lcov
#   - genhtml (included with lcov)
#
# Options:
#   ENABLE_COVERAGE : Enable code coverage instrumentation (gcov/lcov). Default: OFF
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
#   cmake --build build
#   ctest --test-dir build
#   cmake --build build --target coverage         # Generate report
#   cmake --build build --target coverage-open    # Generate report and open it in browser
#   cmake --build build --target coverage-clean   # Remove coverage data
#
# Generated files:
#   - build/coverage/         : HTML report directory
#   - build/coverage.info     : lcov data file
###############################################################################


###############################################################################
# Enable code coverage instrumentation (gcov/lcov)
#
# Usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
#   cmake --build build --target coverage
###############################################################################

option(ENABLE_COVERAGE "Enable code coverage instrumentation (for Debug builds)" OFF)

if(ENABLE_COVERAGE)

    # Recommend debug build for coverage
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING
            "Coverage is enabled but build type is '${CMAKE_BUILD_TYPE}'.\n"
            "For accurate coverage, use Debug: -DCMAKE_BUILD_TYPE=Debug"
        )
    endif()

    # Find lcov executable to generate coverage data
    find_program(LCOV_PATH lcov)
    if(NOT LCOV_PATH)
        message(WARNING "lcov not found. Install with: sudo apt install lcov")
    endif()

    # Find genhtml executable to generate HTML report
    find_program(GENHTML_PATH genhtml)
    if(NOT GENHTML_PATH)
        message(WARNING "genhtml not found. Install with: sudo apt install lcov")
    endif()

    # Find gcov executable to generate coverage data
    find_program(GCOV_PATH gcov)
    if(NOT GCOV_PATH)
        message(WARNING "gcov not found. Install with: sudo apt install gcov")
    endif()

    # Find browser executable to open the HTML report in the default browser
    find_program(BROWSER_EXECUTABLE NAMES
        xdg-open      # Linux (generic)
        firefox
        chromium-browser
        google-chrome
        open          # macOS
    )

    # Create custom coverage targets: coverage, coverage-open, coverage-clean
    if(LCOV_PATH AND GENHTML_PATH)

        # Coverage Target
        # Usage: cmake --build build --target coverage
        add_custom_target(coverage
            COMMENT "Generating code coverage report..."

            # Step 1: Reset coverage counters
            COMMAND ${LCOV_PATH}
                --zerocounters
                --directory ${CMAKE_BINARY_DIR}

            # Step 2: Run all tests
            COMMAND ${CMAKE_CTEST_COMMAND}
                --test-dir ${CMAKE_BINARY_DIR}
                --output-on-failure

            # Step 3: Capture coverage data
            COMMAND ${LCOV_PATH}
                --capture
                --directory ${CMAKE_BINARY_DIR}
                --output-file ${CMAKE_BINARY_DIR}/coverage.info
                --ignore-errors mismatch

            # Step 4: Remove external code (system headers, tests, googletest)
            COMMAND ${LCOV_PATH}
                --remove ${CMAKE_BINARY_DIR}/coverage.info
                '/usr/*'
                '${CMAKE_BINARY_DIR}/_deps/*'
                '${CMAKE_SOURCE_DIR}/*/tests/*'
                --output-file ${CMAKE_BINARY_DIR}/coverage.info
                --ignore-errors unused

            # Step 5: Generate HTML report
            COMMAND ${GENHTML_PATH}
                ${CMAKE_BINARY_DIR}/coverage.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage
                --title "${PROJECT_NAME} Coverage Report"
                --legend
                --show-details

            # Print report location
            COMMAND ${CMAKE_COMMAND} -E echo
                "Coverage report: ${CMAKE_BINARY_DIR}/coverage/index.html"

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            VERBATIM
        )

        message(STATUS "Coverage targets created")
        message(STATUS "  cmake --build build --target coverage       # Generate report")

        # Coverage Open Target
        # Usage: cmake --build build --target coverage-open
        if(BROWSER_EXECUTABLE)
            add_custom_target(coverage-open
                COMMENT "Opening coverage report in browser..."
                COMMAND ${BROWSER_EXECUTABLE}
                    "${CMAKE_BINARY_DIR}/coverage/index.html"
                DEPENDS coverage
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                VERBATIM
            )
            message(STATUS "  cmake --build build --target coverage-open  # Generate + open in browser")
        else()
            message(STATUS "No browser found: 'coverage-open' target unavailable")
        endif()

        # Coverage Clean Target
        # Usage: cmake --build build --target coverage-clean
        add_custom_target(coverage-clean
            COMMENT "Cleaning coverage data..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/coverage.info
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            VERBATIM
        )

        message(STATUS "  cmake --build build --target coverage-clean # Remove coverage data")
        message(STATUS "  Report: build/coverage/index.html")

    else()
        message(WARNING
            "Coverage tools not found. Install lcov:\n"
            "  sudo apt install lcov"
        )
    endif()

    add_enabled_feature("Coverage")

endif()

###############################################################################
# target_enable_coverage(<target>)
#
# Apply coverage instrumentation flags to the specified target.
#
# Arguments:
#   target_name - The CMake target to instrument
#
# Example:
#   add_library(mylib src/mylib.cpp)
#   target_enable_coverage(mylib)
#
# Coverage flags:
#   --coverage     : Enable gcov instrumentation (shorthand for -fprofile-arcs -ftest-coverage)
#   -fprofile-arcs : Generate .gcda files at runtime
#   -ftest-coverage: Generate .gcno files at compile time
###############################################################################

function(target_enable_coverage target_name)

    if(NOT ENABLE_COVERAGE)
        return()
    endif()

    message(STATUS "  Enabling coverage for ${target_name}")

    # Set coverage compile flags
    target_compile_options(${target_name} PRIVATE
        --coverage    # is equivalent to -fprofile-arcs -ftest-coverage
        -O0           # Disable optimization for accurate coverage
        -g            # Debug symbols for line mapping
    )

    # Set coverage link flags
    target_link_options(${target_name} PRIVATE --coverage)

endfunction()