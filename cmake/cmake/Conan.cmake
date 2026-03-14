###############################################################################
# Conan 2.x Package Manager Integration
#
# This module integrates Conan 2.x with CMake using the CMakeDeps and
# CMakeToolchain generators.
#   - Verifies that Conan toolchain was generated
#   - Sets up CMAKE_PREFIX_PATH for find_package()
#   - Provides helper function to find Conan packages
#
# Steps to use Conan with CMake:
#   1. Create a conanfile.txt at the project root
#   2. Run: conan install . --output-folder=build --build=missing
#   3. Configure CMake: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
###############################################################################

print_box("Conan Package Manager")

###############################################################################
# Check if CMAKE_TOOLCHAIN_FILE was provided (indicates conan install was run)
# Reference the variable to suppress "Manually-specified variables were not used"
# warning on reconfigure (toolchain is cached after first run).
###############################################################################

if(CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(WARNING
        "Conan toolchain not provided.\n"
        "Please run: conan install . --output-folder=build --build=missing\n"
        "Then reconfigure with: make -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake"
    )
endif()

###############################################################################
# find_conan_package(<package_name> [REQUIRED] [COMPONENTS comp1 comp2 ...])
#
# Wrapper around find_package() for Conan dependencies.
# Provides consistent error messages and logging.
###############################################################################

function(find_conan_package PACKAGE_NAME)
    cmake_parse_arguments(ARG "REQUIRED" "" "COMPONENTS" ${ARGN})

    if(ARG_COMPONENTS)
        find_package(${PACKAGE_NAME} COMPONENTS ${ARG_COMPONENTS})
    else()
        find_package(${PACKAGE_NAME})
    endif()

    if(${PACKAGE_NAME}_FOUND)
        message(STATUS "Conan: Found ${PACKAGE_NAME}")
    elseif(ARG_REQUIRED)
        message(FATAL_ERROR
            "Conan: ${PACKAGE_NAME} not found.\n"
            "Make sure it's listed in conanfile.txt and run:\n"
            "  conan install . --output-folder=${CMAKE_BINARY_DIR} --build=missing"
        )
    else()
        message(STATUS "Conan: ${PACKAGE_NAME} not found (optional)")
    endif()
endfunction()