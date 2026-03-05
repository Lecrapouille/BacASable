# CMake C++ Infrastructure

A reusable CMake infrastructure for C++ multi-module projects. It provides high-level functions to declare libraries and executables with automatic source discovery, strict compiler warnings, precompiled headers, sanitizers, code coverage, and more.

This infrastructure expects each module to follow a standard directory layout:

```
<module>/
├── CMakeLists.txt
├── include/<module>/   # Public headers
├── src/                # Sources and private headers
├── pch/                # Custom precompiled header (optional)
│   └── pch.hpp
└── tests/              # GoogleTest unit tests
```

The modules included in this repository (`kinematics`, `odometry`, `robot_controller`) are **demonstration examples only**. They exist to illustrate how to use the CMake functions and are not intended to be reused as-is. The demo uses [mp-units](https://mpusz.github.io/mp-units/) for SI unit handling (velocities in m/s, rad/s).

---

## CMake Files

All infrastructure files live in the `cmake/` directory. Include `ProjectBootstrap.cmake` once from your root `CMakeLists.txt` to enable everything.

| File | Purpose |
|------|---------|
| `ProjectBootstrap.cmake` | Single entry point. Sets C++ standard, enables `compile_commands.json`, declares all build options, and includes every other module in the correct order. |
| `Conan.cmake` | Conan 2.x package manager integration. Finds packages installed via `conan install`. |
| `CompilerWarnings.cmake` | Configures default build type (`Release`), enables Position Independent Code (PIC), and provides `target_set_warnings()` for strict warning flags. |
| `Utilities.cmake` | Main API: `add_module_library()`, `add_module_executable()`, `install_module_dependencies()`. |
| `PCH.cmake` | Manages precompiled headers. Provides a global auto-generated PCH and support for custom per-module PCH files. |
| `Testing.cmake` | Declares `BUILD_TESTING` option, fetches GoogleTest, and enables CTest. |
| `FetchGoogleTest.cmake` | Downloads GoogleTest via `FetchContent`. Provides `GTest::gtest_main` and related targets. |
| `FetchBackwardCpp.cmake` | Downloads backward-cpp for readable stack traces on crashes. Provides `Backward::Backward` target. |
| `Sanitizers.cmake` | Provides `target_enable_sanitizers()` for AddressSanitizer, UndefinedBehaviorSanitizer, and ThreadSanitizer. |
| `Coverage.cmake` | Provides `target_enable_coverage()` and creates `coverage`, `coverage-open`, `coverage-clean` targets for gcov/lcov reports. |
| `DebugSymbols.cmake` | Declares `ENABLE_SPLIT_DWARF` and `ENABLE_STRIP`. Provides functions for split DWARF and symbol extraction. |
| `Doxygen.cmake` | Creates a `docs` target for generating HTML API documentation with Doxygen. |

---

## Quick Start

### Prerequisites

- CMake 3.20+
- C++20 compatible compiler (GCC 11+, Clang 14+)
- Conan 2.x package manager

### Install Conan (if not already installed)

```bash
pip install conan
conan profile detect
```

### Build the Project

```bash
# 1. Install dependencies with Conan
conan install . --output-folder=build --build=missing -s compiler.cppstd=20

# 2. Configure and build
cmake --preset release
cmake --build build

# 3. Run tests
ctest --preset release

# 4. Run the demo
./build/robot_controller/robot_controller
```

### Debug Build

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Debug
cmake --preset debug
cmake --build build
```

Build options are passed during the configure step using `-D<OPTION>=<VALUE>`. See the **Build Options** section below for all available options.

---

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Build configuration: `Debug`, `Release`, `MinSizeRel`, `RelWithDebInfo` |
| `BUILD_TESTING` | `ON` | Build unit tests with GoogleTest |
| `ENABLE_WERROR` | `OFF` | Treat compiler warnings as errors (`-Werror`) |
| `ENABLE_LTO` | `OFF` | Link Time Optimization (Release builds only) |
| `ENABLE_SPLIT_DWARF` | `OFF` | Split debug info into `.dwo` files for faster linking |
| `ENABLE_STRIP` | `OFF` | Strip symbols from Release binaries and extract `.debug` files |
| `ENABLE_ASAN` | `OFF` | AddressSanitizer: detects buffer overflows, use-after-free, memory leaks |
| `ENABLE_UBSAN` | `OFF` | UndefinedBehaviorSanitizer: detects integer overflow, null dereferences |
| `ENABLE_TSAN` | `OFF` | ThreadSanitizer: detects data races and deadlocks (incompatible with ASAN) |
| `ENABLE_COVERAGE` | `OFF` | Code coverage instrumentation with gcov/lcov |
| `ENABLE_DOXYGEN` | `OFF` | Generate HTML API documentation with Doxygen |
| `FETCH_BACKWARD_CPP` | `ON` | Fetch backward-cpp for crash stack traces |

---

## API Reference

### `add_module_library()`

Creates a library module (static or shared) with automatic source discovery, compiler warnings, PCH, and unit tests.

```cmake
add_module_library(<name>
    [SHARED]
    [NO_INSTALL]
    [PCH <path>]
    [PUBLIC_DEPENDENCIES <dep1> <dep2> ...]
    [PRIVATE_DEPENDENCIES <dep1> <dep2> ...]
    [TEST_DEPENDENCIES <dep1> <dep2> ...]
)
```

#### Arguments

| Argument | Type | Required | Description |
|----------|------|----------|-------------|
| `<name>` | positional | **Yes** | Name of the library target |
| `SHARED` | flag | No | Create a shared library (`.so`) instead of static (`.a`). Configures RPATH automatically. |
| `PUBLIC_DEPENDENCIES` | list | No | Libraries linked with `PUBLIC` visibility. Propagated to consumers of this library. |
| `PRIVATE_DEPENDENCIES` | list | No | Libraries linked with `PRIVATE` visibility. Used only internally. |
| `TEST_DEPENDENCIES` | list | No | Additional libraries linked to the `<name>_tests` test executable. |
| `PCH` | single value | No | Path to a custom PCH header relative to the module directory (e.g., `pch/pch.hpp`). Replaces the global PCH for this target. |
| `NO_INSTALL` | flag | No | Skip installation rules for this target. |

#### Automatic Behaviors

- **Source discovery**: Globs `src/*.cpp`, `src/*.c`, `include/*.h`, `include/*.hpp` with `CONFIGURE_DEPENDS`.
- **Library type**: Creates a `STATIC` library by default, or `SHARED` if the flag is provided.
- **RPATH (shared only)**: Configures `$ORIGIN/../lib` RPATH so executables find the `.so` at runtime.
- **Alias target**: Creates `${PROJECT_NAME}::<name>` alias for safe usage in `target_link_libraries()`.
- **Include directories**: `include/` is `PUBLIC`, `src/` is `PRIVATE`.
- **Compiler warnings**: Applies strict warnings via `target_set_warnings()`.
- **Sanitizers**: Applies ASAN/UBSAN/TSAN if enabled globally.
- **Coverage**: Instruments for coverage if `ENABLE_COVERAGE` is `ON`.
- **PCH**: Uses the global PCH unless `PCH` argument is provided.
- **Installation**: Installs to `lib/` and `include/` under component `devel` (unless `NO_INSTALL`).
- **Tests**: Creates `<name>_tests` executable if `BUILD_TESTING` is `ON` and `tests/*.cpp` files exist.

#### Examples

```cmake
# Demo project example (kinematics with SI units and custom PCH)
add_module_library(kinematics
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
)

# Library with custom PCH
add_module_library(database
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        nlohmann_json::nlohmann_json
    PRIVATE_DEPENDENCIES
        SQLite::SQLite3
    TEST_DEPENDENCIES
        testutils
)

# Shared library
add_module_library(mysharedlib
    SHARED
    PUBLIC_DEPENDENCIES SomeLib::SomeLib
)
```

---

### `add_module_executable()`

Creates an executable module with automatic source discovery, compiler warnings, PCH, and optional unit tests.

```cmake
add_module_executable(<name>
    [NO_INSTALL]
    [PCH <path>]
    [DEPENDENCIES <dep1> <dep2> ...]
    [TEST_DEPENDENCIES <dep1> <dep2> ...]
)
```

#### Arguments

| Argument | Type | Required | Description |
|----------|------|----------|-------------|
| `<name>` | positional | **Yes** | Name of the executable target |
| `DEPENDENCIES` | list | No | Libraries linked with `PRIVATE` visibility. |
| `TEST_DEPENDENCIES` | list | No | Libraries for the test executable. **Tests are only created if this argument is provided.** |
| `PCH` | single value | No | Path to a custom PCH header relative to the module directory (e.g., `pch/pch.hpp`). Replaces the global PCH for this target. |
| `NO_INSTALL` | flag | No | Skip installation rules for this target. |

#### Automatic Behaviors

- **Source discovery**: Globs `src/*.cpp`, `src/*.c`, `include/*.h`, `include/*.hpp`, `src/*.h`, `src/*.hpp`.
- **Include directories**: Both `include/` and `src/` are `PRIVATE`.
- **Compiler warnings**: Applies strict warnings via `target_set_warnings()`.
- **Sanitizers**: Applies ASAN/UBSAN/TSAN if enabled globally.
- **Coverage**: Instruments for coverage if `ENABLE_COVERAGE` is `ON`.
- **PCH**: Uses the global PCH unless `PCH` argument is provided.
- **Installation**: Installs to `bin/` under component `runtime` (unless `NO_INSTALL`).
- **Tests**: Creates `<name>_tests` executable only if `TEST_DEPENDENCIES` is provided. The test executable recompiles all sources from `src/` **except** `main.cpp`, allowing you to test application logic without the entry point.

#### Example

```cmake
# Demo project example (robot_controller with custom PCH)
add_module_executable(robot_controller
    PCH pch/pch.hpp
    DEPENDENCIES
        kinematics
        odometry
    TEST_DEPENDENCIES
        kinematics
        odometry
)
```

---

### `install_module_dependencies()`

Deploys shared library dependencies alongside an executable at install time.

```cmake
install_module_dependencies(<name>)
```

#### Arguments

| Argument | Type | Required | Description |
|----------|------|----------|-------------|
| `<name>` | positional | **Yes** | Name of an `EXECUTABLE` target |

#### Behavior

At `cmake --install` time, this function:
1. Analyzes the executable's runtime dependencies using `file(GET_RUNTIME_DEPENDENCIES)`.
2. Excludes system libraries (`/lib/*`, `/usr/lib/*`).
3. Copies resolved dependencies to `<prefix>/lib/`.

Useful for creating self-contained distribution packages with shared libraries.

#### Example

```cmake
add_module_executable(myapp
    DEPENDENCIES mysharedlib
)
install_module_dependencies(myapp)
```

---

## Precompiled Headers (PCH)

### Global PCH

By default, all targets use an auto-generated global PCH located at `${CMAKE_BINARY_DIR}/generated/global_pch.hpp`. This file contains commonly used STL headers:

- Containers: `<vector>`, `<map>`, `<set>`, `<unordered_map>`, `<unordered_set>`, `<array>`, `<deque>`, `<list>`
- Strings: `<string>`, `<string_view>`
- Memory: `<memory>`
- Utilities: `<algorithm>`, `<functional>`, `<optional>`, `<tuple>`, `<utility>`, `<variant>`
- I/O: `<iostream>`, `<sstream>`
- Misc: `<chrono>`, `<cstddef>`, `<cstdint>`, `<stdexcept>`, `<type_traits>`

The global PCH is compiled once and shared across all targets via CMake's `REUSE_FROM` mechanism.

### Custom PCH

For modules that use heavy third-party headers (mp-units, Qt, Boost, etc.), you can specify a custom PCH:

```cmake
add_module_library(mymodule
    PCH pch/pch.hpp
)
```

**Important:** A custom PCH **completely replaces** the global PCH for that target.

#### Including the Global PCH

You can include the global PCH in your custom PCH to get all STL headers plus your module-specific headers:

```cpp
// mymodule/pch/pch.hpp
#pragma once

// Include the global PCH (all STL headers)
#include "global_pch.hpp"

// Add module-specific heavy headers
#include <mp-units/systems/si.h>
#include <boost/asio.hpp>
#include <QApplication>
```

The `#include "global_pch.hpp"` works because the generated directory (`${CMAKE_BINARY_DIR}/generated/`) is automatically added to the include path when using a custom PCH.

#### Standalone Custom PCH

Alternatively, you can create a fully standalone custom PCH without including the global one:

```cpp
// mymodule/pch/pch.hpp
#pragma once

// STL headers (declare only what you need)
#include <vector>
#include <string>
#include <memory>
#include <map>

// Heavy third-party headers
#include <boost/asio.hpp>
```

### When to Use Custom PCH

| Situation | Recommendation |
|-----------|----------------|
| Standard module using only STL | Use global PCH (default) |
| Module with mp-units, Qt, Boost, or other heavy headers | Use custom PCH |
| Header-only module | No PCH needed |

---

## Common Recipes

```bash
# AddressSanitizer (memory errors)
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Debug
cmake --preset asan && cmake --build build && ctest --preset asan

# UndefinedBehaviorSanitizer
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Debug
cmake --preset ubsan && cmake --build build && ctest --preset ubsan

# ThreadSanitizer (data races)
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Debug
cmake --preset tsan && cmake --build build && ctest --preset tsan

# Code coverage
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Debug
cmake --preset coverage && cmake --build build && ctest --preset coverage
cmake --build build --target coverage-open
# Report: build/coverage/index.html

# CI build (warnings as errors, no tests)
conan install . --output-folder=build --build=missing -s compiler.cppstd=20
cmake --preset ci && cmake --build build

# Release with LTO and stripped symbols
conan install . --output-folder=build --build=missing -s compiler.cppstd=20
cmake --preset release -DENABLE_LTO=ON -DENABLE_STRIP=ON
cmake --build build

# Generate API documentation
conan install . --output-folder=build --build=missing -s compiler.cppstd=20
cmake --preset release -DENABLE_DOXYGEN=ON
cmake --build build --target docs
# Output: build/docs/html/index.html
```

---

## Dependencies

### Conan Packages

Dependencies are managed via Conan 2.x. The `conanfile.txt` declares:

| Package | Version | Purpose |
|---------|---------|---------|
| [mp-units](https://github.com/mpusz/mp-units) | 2.3.0 | SI unit library for compile-time dimensional analysis |

To add more dependencies, edit `conanfile.txt`:

```ini
[requires]
mp-units/2.3.0
boost/1.84.0

[generators]
CMakeDeps
CMakeToolchain
```

Then re-run `conan install` and use `find_package()` in your CMake files.

### Fetched Automatically

The following libraries are fetched automatically via `FetchContent` when building:

| Library | Version | Purpose |
|---------|---------|---------|
| [GoogleTest](https://github.com/google/googletest) | v1.14.0 | Unit testing framework (when `BUILD_TESTING=ON`) |
| [backward-cpp](https://github.com/bombela/backward-cpp) | v1.6 | Stack traces on crashes (when `FETCH_BACKWARD_CPP=ON`) |

---

## Installation

The installation step copies built artifacts to a destination directory (default: `/usr/local` on Linux).

### Full Installation

```bash
cmake --install build --prefix /usr/local
```

This installs:
- Executables to `<prefix>/bin/`
- Libraries (`.a` or `.so`) to `<prefix>/lib/`
- Public headers to `<prefix>/include/`

### Component-Based Installation

Targets are automatically assigned to components based on their type:

| Component | Contents | Created by |
|-----------|----------|------------|
| `runtime` | Executables | `add_module_executable()` |
| `devel` | Libraries (`.a` / `.so`) and public headers | `add_module_library()` |

This allows installing only what you need:

```bash
# Install executables only (for deployment)
cmake --install build --component runtime

# Install development files only (for SDK/library distribution)
cmake --install build --component devel
```

Use `NO_INSTALL` flag in `add_module_library()` or `add_module_executable()` to exclude a target from installation.
