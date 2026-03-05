# ============================================================================
# Testing.cmake - Unit Testing Configuration
# ============================================================================
#
# This module configures unit testing with GoogleTest framework.
# It fetches GoogleTest via FetchContent and enables CTest integration.
#
# Usage:
#   In root CMakeLists.txt:
#     option(BUILD_TESTING "Build unit tests" ON)
#     include(Testing)
#
#   Run tests:
#     ctest --test-dir build
#     ctest --test-dir build --output-on-failure
#     ctest --test-dir build -L unit           # Run only 'unit' labeled tests
#
# ============================================================================

# Define BUILD_TESTING early so it can be used in StandardSettings
# and other modules that are included before add_subdirectory().
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)

    print_box("GoogleTest Framework")

    # ------------------------------------------------------------------
    # Fetch GoogleTest
    # ------------------------------------------------------------------
    # Download and configure GoogleTest framework.
    # After this, the following targets are available:
    #   - GTest::gtest       : Core library
    #   - GTest::gtest_main  : With main() function
    #   - GTest::gmock       : GoogleMock
    #   - GTest::gmock_main  : GoogleMock with main()

    include(FetchGoogleTest)

    # ------------------------------------------------------------------
    # Enable CTest
    # ------------------------------------------------------------------
    # This enables the 'test' target and allows running tests via ctest.
    # Tests are discovered at build time using gtest_discover_tests().

    enable_testing()

    message(STATUS "GoogleTest: Enabled")
    message(STATUS "Run tests: ctest --test-dir build")

    # Register for configuration summary
    list(APPEND _ENABLED_FEATURES "Tests")
    set(_ENABLED_FEATURES "${_ENABLED_FEATURES}" CACHE INTERNAL "")

endif()
