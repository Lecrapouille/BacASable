# ============================================================================
# Conan.cmake - Conan Package Manager Integration
# ============================================================================
#
# This module integrates Conan 2.x with CMake using the CMakeDeps and
# CMakeToolchain generators.
#
# Usage:
#   1. Create a conanfile.txt at the project root
#   2. Run: conan install . --output-folder=build --build=missing
#   3. Configure CMake: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
#
# This module:
#   - Verifies that Conan toolchain was generated
#   - Sets up CMAKE_PREFIX_PATH for find_package()
#   - Provides helper function to find Conan packages
#
# ============================================================================

print_box("Conan Package Manager")

# Check if CMAKE_TOOLCHAIN_FILE was provided (indicates conan install was run)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(WARNING
        "Conan toolchain not provided.\n"
        "Please run: conan install . --output-folder=build --build=missing\n"
        "Then reconfigure with: cmake --preset conan-release (or conan-debug)"
    )
endif()

# ============================================================================
# find_conan_package(<package_name> [REQUIRED] [COMPONENTS comp1 comp2 ...])
# ============================================================================
# Wrapper around find_package() for Conan dependencies.
# Provides consistent error messages and logging.
#
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


# ============================================================================
# Find Conan Packages
# ============================================================================
# Import packages defined in conanfile.txt

find_conan_package(mp-units REQUIRED)
