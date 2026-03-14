###############################################################################
# Unit Testing Configuration
#
# Configures GoogleTest (via Conan) and enables CTest integration.
#
# Options:
#   BUILD_TESTING: Build unit tests (default: ON)
#
# Available targets after inclusion:
#   GTest::gtest       - Core library
#   GTest::gtest_main  - With main()
#   GTest::gmock       - GoogleMock
#   GTest::gmock_main  - GoogleMock with main()
#
# Usage:
#   cmake -S . -B build -DBUILD_TESTING=ON
#   ctest --test-dir build
#   ctest --test-dir build --output-on-failure
#   ctest --test-dir build -L unit           # Run only 'unit' labeled tests
###############################################################################


option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)

    print_box("GoogleTest Framework")

    find_conan_package(GTest REQUIRED)

    include(GoogleTest)

    enable_testing()

    message(STATUS "GoogleTest: Enabled")
    message(STATUS "Run tests: ctest --test-dir build")

    add_enabled_feature("Tests")

    ###########################################################################
    # discover_tests_for_target(<target> [PROPERTIES prop1 val1 ...])
    #
    # Wraps gtest_discover_tests with default properties (TIMEOUT 120, LABELS unit).
    # Extra PROPERTIES can be appended.
    ###########################################################################

    function(discover_tests_for_target target)
        cmake_parse_arguments(ARG "" "" "PROPERTIES" ${ARGN})
        gtest_discover_tests(${target}
            PROPERTIES
                TIMEOUT 120
                LABELS "unit"
                ${ARG_PROPERTIES}
        )
    endfunction()

endif()
