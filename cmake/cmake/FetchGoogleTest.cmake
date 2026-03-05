# ============================================================================
# FetchGoogleTest.cmake - GoogleTest Framework Integration
# ============================================================================
#
# This module downloads and configures GoogleTest using CMake's FetchContent.
# FetchContent downloads dependencies at configure time, making them available
# as CMake targets in the build.
#
# After including this module, the following targets are available:
#   - GTest::gtest        : GoogleTest core library
#   - GTest::gtest_main   : GoogleTest with main() function
#   - GTest::gmock        : GoogleMock library
#   - GTest::gmock_main   : GoogleMock with main() function
#
# Usage in test CMakeLists.txt:
#   target_link_libraries(my_tests PRIVATE GTest::gtest_main)
#
# This module also includes GoogleTest's CMake helper module which provides:
#   - gtest_discover_tests() : Auto-discover and register tests with CTest
#
# ============================================================================

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# GoogleTest Configuration
# gtest_force_shared_crt: Prevents GoogleTest from overriding the runtime
# library settings. Important for consistent linking across the project.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)