cmake_minimum_required(VERSION 3.14)

project(Container
    VERSION 1.0.0
    DESCRIPTION "Block-allocated containers optimized for large datasets"
    LANGUAGES CXX
)

# ==============================================================================
# C++20 Requirements
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Compiler Warnings (applied only to our targets, not dependencies)
# ==============================================================================
set(CONTAINER_COMPILE_OPTIONS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CONTAINER_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
    )
elseif(MSVC)
    set(CONTAINER_COMPILE_OPTIONS /W4 /WX)
endif()

# ==============================================================================
# Header-only Library
# ==============================================================================
add_library(container INTERFACE)
target_include_directories(container INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# Testing (optional, enabled by default)
# ==============================================================================
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()

    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    # Include GoogleTest module for gtest_discover_tests
    include(GoogleTest)

    # Test for Set
    add_executable(test_set tests/test_set.cpp)
    target_link_libraries(test_set PRIVATE container GTest::gtest_main)
    target_compile_options(test_set PRIVATE ${CONTAINER_COMPILE_OPTIONS})
    gtest_discover_tests(test_set)

    # Test for Collection
    add_executable(test_collection tests/test_collection.cpp)
    target_link_libraries(test_collection PRIVATE container GTest::gtest_main)
    target_compile_options(test_collection PRIVATE ${CONTAINER_COMPILE_OPTIONS})
    gtest_discover_tests(test_collection)
endif()

# ==============================================================================
# Installation
# ==============================================================================
include(GNUInstallDirs)

install(TARGETS container
    EXPORT containerTargets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT containerTargets
    FILE containerTargets.cmake
    NAMESPACE container::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/container
)

# Generate package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/containerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/containerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/containerConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/container
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/containerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/containerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/container
)
