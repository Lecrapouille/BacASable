cmake_minimum_required(VERSION 3.14)
project(TestMock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main project
add_executable(main
    main.cpp
    MainClass.cpp
    Publisher.cpp
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Download Google Test (and Google Mock) using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# Google Test requires Google Mock (included in googletest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the tests
# IMPORTANT: The test/ directory comes FIRST in include paths
# so our mock Publisher.h is found before the real one
add_executable(test_mainclass
    test/test_mainclass.cpp
    # We do NOT include Publisher.cpp here!
    # MainClass.cpp is included directly in test_mainclass.cpp
)

# test/ directory must be FIRST to intercept Publisher.h
target_include_directories(test_mainclass PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/test  # Our mock Publisher.h
    ${CMAKE_CURRENT_SOURCE_DIR}       # Other headers
)

target_link_libraries(test_mainclass
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    pthread
)

# Add the test to CTest
enable_testing()
add_test(NAME MainClassTests COMMAND test_mainclass)
