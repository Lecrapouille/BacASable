# 🏗️ CMake C++ Infrastructure

A reusable CMake infrastructure for C++ multi-module projects. It provides high-level functions to declare libraries and executables with automatic source discovery, strict compiler warnings, precompiled headers, sanitizers, code coverage, doxygen, stack trace (backward-cpp), unit tests (GoogleTest) and more. Dependencies are managed via Conan. Two single functions do all the work: `library()`, `executable()`.

This infrastructure expects each module to follow a standard directory layout (sources and headers are discovered as `*.c`, `*.cpp`, `*.h`, `*.hpp` only):

```bash
<module>/
├── CMakeLists.txt
├── include/<module>/   # Public headers (or include/<subdir>/ when using SUBDIRECTORIES)
├── src/                # Sources and private headers
├── mocks/              # Mock implementations (optional)
├── pch/                # Custom precompiled header (optional)
│   └── pch.hpp
└── tests/              # GoogleTest unit tests (optional)
```

The demo libraries (`kinematics`, `odometry`, `robot_controller`) live under [`robot/`](robot/) in a **multi-subdirectory** layout (`include/<name>/`, `src/<name>/`, …) with one `library()` call per target and the `SUBDIRECTORIES` keyword. They are **examples only**. The demo uses [mp-units](https://mpusz.github.io/mp-units/) for SI unit handling (velocities in m/s, rad/s) installed from Conan.

---

## 🗂️ CMake Files

All infrastructure files live in the `cmake/` directory. Include `ProjectBootstrap.cmake` once from your root `CMakeLists.txt` to enable everything. `ProjectBootstrap.cmake` is the single entry point including other cmake files.

| File | Purpose |
|------|---------|
| `Module.cmake` | Main API: `library()`, `executable()`, and private `_module_*` helpers (glob, banner, configure, gtest). |
| `Conan.cmake` | Conan 2.x integration. Provides `find_conan_package()` wrapper. |
| `Compiler.cmake` | Default build type, PIC, LTO, debug flags, `target_set_warnings()`. |
| `PCH.cmake` | Precompiled headers: global auto-generated PCH and custom per-module PCH support. |
| `Testing.cmake` | `BUILD_TESTING` option, GoogleTest (via Conan), CTest integration. |
| `Stacktrace.cmake` | Crash stack traces via backward-cpp (Conan). Auto-linked on executables. |
| `Summary.cmake` | Automatic project summary box, configuration summary, dependency graph (`build/dependencies.dot`). |
| `Sanitizers.cmake` | `target_enable_sanitizers()` for ASAN, UBSAN, TSAN. |
| `Coverage.cmake` | `target_enable_coverage()` and `coverage`/`coverage-open`/`coverage-clean` targets. |
| `DebugSymbols.cmake` | Split DWARF (`.dwo` files) and separated debug symbols at install time. |
| `Doxygen.cmake` | `docs` target for HTML API documentation with Doxygen. |
| `PrettyPrint.cmake` | Formatted console output (boxed titles, key-value sections). |

## ⚙️ Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Build configuration: `Debug`, `Release`, `MinSizeRel`, `RelWithDebInfo` |
| `BUILD_TESTING` | `ON` | Build unit tests with GoogleTest |
| `ENABLE_WERROR` | `OFF` | Treat compiler warnings as errors |
| `ENABLE_STACKTRACE` | `ON` | Enable backward-cpp crash stack traces |
| `ENABLE_LTO` | `OFF` | Link Time Optimization (Release builds only) |
| `ENABLE_SPLIT_DWARF` | `OFF` | Split debug info into `.dwo` files for faster linking |
| `ENABLE_ASAN` | `OFF` | AddressSanitizer: detects buffer overflows, use-after-free, memory leaks |
| `ENABLE_UBSAN` | `OFF` | UndefinedBehaviorSanitizer: detects integer overflow, null dereferences |
| `ENABLE_TSAN` | `OFF` | ThreadSanitizer: detects data races and deadlocks (incompatible with ASAN) |
| `ENABLE_COVERAGE` | `OFF` | Code coverage instrumentation with gcov/lcov |
| `ENABLE_DOXYGEN` | `OFF` | Generate HTML API documentation with Doxygen |

---

## 🛠️ Prerequisites

- CMake 3.20+
- C++20 compatible compiler (GCC 11+, Clang 14+)
- Conan 2.x package manager

All dependencies are managed via Conan 2.x. To install Conan (if not already installed)

```bash
pip install conan
conan profile detect
```

The `conanfile.txt`, at the root of the project, declares:

| Package | Version | Purpose |
|---------|---------|---------|
| [mp-units](https://github.com/mpusz/mp-units) | 2.3.0 | SI unit library for compile-time dimensional analysis |
| [GoogleTest](https://github.com/google/googletest) | 1.15.0 | Unit testing framework (when `BUILD_TESTING=ON`) |
| [backward-cpp](https://github.com/bombela/backward-cpp) | 1.6 | Crash stack traces (when `ENABLE_STACKTRACE=ON`) |

To add more dependencies, edit `conanfile.txt`:

```ini
[requires]
mp-units/2.3.0
gtest/1.15.0
backward-cpp/1.6
spdlog/1.14.1         # example: add your own packages

[generators]
CMakeDeps
CMakeToolchain
```

Then re-run `conan install . --output-folder=build --build=missing -s compiler.cppstd=20` and use `find_conan_package()` or `find_package()` in your CMake files.

---

## 🚀 Compilation

### Build the Project (release)

```bash
# 1. Install dependencies with Conan
conan install . --output-folder=build --build=missing -s compiler.cppstd=20

# 2. Configure and build
cmake --preset release
cmake --build build -j$(nproc)

# 3. Run tests
ctest --preset release

# 4. Run the demo
./build/application/application
```

### Build the Project (debug)

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Debug
cmake --preset debug
cmake --build build -j$(nproc)
./build/application/application
```

### Common Recipes

The `CMakePresets.json` defines some common recipes:

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

# Release with LTO and stripped install
conan install . --output-folder=build --build=missing -s compiler.cppstd=20
cmake --preset release -DENABLE_LTO=ON
cmake --build build
cmake --install build --strip

# Generate API documentation
conan install . --output-folder=build --build=missing -s compiler.cppstd=20
cmake --preset release -DENABLE_DOXYGEN=ON
cmake --build build --target docs
# Output: build/docs/html/index.html
```

---

## 📥 Installation

By default, `cmake --install build` deploys to the local `install/` directory (set via `CMAKE_INSTALL_PREFIX` in the root `CMakeLists.txt`). For each module, this installs:

- Executables to `install/bin/`
- Libraries (`.a` or `.so`) to `install/lib/`
- Public headers to `install/include/`
- Runtime shared library dependencies to `install/lib/` (auto-deployed for executables)
- Debug symbols to `.debug/` subdirectories (Debug/RelWithDebInfo builds only)
- Use `NO_INSTALL` flag in `library()` or `executable()` to exclude a target from installation.

For a full installation, you have to override the install location:

```bash
cmake --install build --prefix /opt/myapp
```

### 🏷️ Stripped Release Install

For Release builds, use the `--strip` flag to install stripped binaries (smaller size, no symbol tables):

```bash
cmake --install build --strip
```

### 🐞 Separated Debug Symbols (Debug/RelWithDebInfo)

For Debug and RelWithDebInfo builds, `cmake --install` automatically separates debug symbols from installed binaries. The install tree looks like:

```
install/
├── bin/
│   ├── application             # Stripped binary (like Release)
│   └── .debug/
│       └── application.debug   # Debug symbols only
├── lib/                        # Libraries + runtime dependencies
└── include/                    # Public headers
```

This provides the best of both worlds: compact binaries suitable for deployment, and separate `.debug` files for post-mortem debugging.

The `.debug/` subdirectory follows the standard Linux convention. GDB automatically searches `<binary_dir>/.debug/` when resolving `gnu-debuglink` references, so **no extra configuration is needed**:

```bash
gdb ./install/bin/application
# GDB auto-loads install/bin/.debug/application.debug via gnu-debuglink
```

> **Note:** When using `cmake --install build --strip` on a Debug build, the automatic debug symbol separation is skipped (since CMake strips the binary before our code can extract symbols). Use the plain `cmake --install build` for debug builds to get separated symbols.

### 🧩 Component-Based Installation

Targets are automatically assigned to components based on their type:

| Component | Contents | Created by |
|-----------|----------|------------|
| `runtime` | Executables + runtime shared library dependencies | `executable()` |
| `devel` | Libraries (`.a` / `.so`) and public headers | `library()` |

This allows installing only what you need:

```bash
# Install executables only (for deployment)
cmake --install build --component runtime

# Install development files only (for SDK/library distribution)
cmake --install build --component devel
```

---

---

## 🧪 Mock System

The infrastructure provides a first-class mock system that works **without virtual functions**, **without `#define`/`#undef` tricks**, and **without touching the real headers**. It is based on the *delegate-to-static-singleton* pattern.

### How It Works

Each mockable library `foo` has three files:

```cpp
include/foo/Foo.h          ← Real class declaration (unchanged in tests)
mocks/foo/FooMock.h        ← GoogleMock struct mirroring Foo's public API
mocks/foo/FooMock.cpp      ← Bridge: implements the real class, delegates to the mock
```

**Real header** — untouched, declares the production class with non-virtual methods:

```cpp
// include/kinematics/DifferentialDrive.h
class DifferentialDrive {
public:
    WheelVelocities twist_to_wheels(const Twist&) const;
    Twist           wheels_to_twist(const WheelVelocities&) const;
};
```

**Mock header** — a separate struct with `MOCK_METHOD` and a static singleton:

```cpp
// mocks/kinematics/DifferentialDriveMock.h
struct DifferentialDriveMock {
    DifferentialDriveMock()  { instance_ = this; }   // RAII registration
    ~DifferentialDriveMock() { instance_ = nullptr; }

    MOCK_METHOD(WheelVelocities, twist_to_wheels, (const Twist&), (const));
    MOCK_METHOD(Twist, wheels_to_twist, (const WheelVelocities&), (const));

    static DifferentialDriveMock* mock() { return instance_; }
private:
    static inline DifferentialDriveMock* instance_ = nullptr;
};
```

**Bridge `.cpp`** — implements the real class's methods, forwards to the mock when active:

```cpp
// mocks/kinematics/DifferentialDriveMock.cpp
#include "DifferentialDriveMock.h"   // already includes DifferentialDrive.h

WheelVelocities DifferentialDrive::twist_to_wheels(const Twist& twist) const {
    if (auto* m = DifferentialDriveMock::mock())
        return m->twist_to_wheels(twist);
    return {};   // fallback when no mock is active
}
```

### CMake Integration

`library()` automatically discovers `mocks/` and creates a `<name>_mock` static library:

```
kinematics        ← production lib  (src/kinematics/*.cpp)
kinematics_mock   ← mock bridge lib (mocks/kinematics/DifferentialDriveMock.cpp)
```

The mock lib exposes the `mocks/` directory as a public include root, so tests can
`#include "kinematics/DifferentialDriveMock.h"`.

Executable tests use `TEST_DEPENDENCIES` to swap the real lib for the mock bridge:

```cmake
executable(application
    DEPENDENCIES      robot_controller          # production build
    TEST_DEPENDENCIES robot_controller_mock     # test build uses the bridge
)
```

### Singleton Lifecycle

Constructing a `*Mock` object activates the singleton (`instance_ = this`). Destroying it
resets to `nullptr`. No `SetUp()`/`TearDown()` boilerplate is needed — just declare the
mock as a fixture member and it is automatically active for the duration of each test:

```cpp
class MyTest : public ::testing::Test {
    RobotControllerMock mock_;   // active from construction to destruction
    RobotController     ctrl_{...};
};
```

### Writing Tests with `EXPECT_CALL`

```cpp
// Exact argument matching (requires operator== on structs)
TEST_F(MyTest, StepForwardsExactTwist) {
    kinematics::Twist cmd{0.5 * (m/s), 0.3 * (rad/s)};
    EXPECT_CALL(mock_, step(cmd)).Times(1);
    ctrl_.step(cmd);
}

// Call-count verification
TEST_F(MyTest, MultipleStepsAreCounted) {
    EXPECT_CALL(mock_, step(::testing::_)).Times(10);
    for (int i = 0; i < 10; ++i) ctrl_.step({});
}

// Return-value injection
TEST_F(MyTest, PoseReturnsMockedValue) {
    static const odometry::Pose2D expected{1.0 * m, 2.0 * m, 0.5 * rad};
    EXPECT_CALL(mock_, pose()).WillOnce(::testing::ReturnRef(expected));
    EXPECT_DOUBLE_EQ(ctrl_.pose().x.numerical_value_in(m), 1.0);
}

// Call-order enforcement
TEST_F(MyTest, StepBeforePose) {
    static const odometry::Pose2D after{0.5 * m, 0.0 * m, 0.0 * rad};
    ::testing::InSequence seq;
    EXPECT_CALL(mock_, step(::testing::_));
    EXPECT_CALL(mock_, pose()).WillOnce(::testing::ReturnRef(after));
    ctrl_.step({0.5 * (m/s), 0.0 * (rad/s)});
    ctrl_.pose();
}
```

### Limitations and Trade-offs

| Criterion | This pattern | Traditional virtual mock |
|-----------|-------------|--------------------------|
| No virtual functions required | ✓ | ✗ |
| No `#define`/`#undef` tricks | ✓ | ✓ |
| Inline methods in headers | ✗ (not interceptable) | ✓ |
| Compiler catches missing bridge methods | ✓ (link error) | ✓ |
| Compiler catches missing `MOCK_METHOD` | ✗ (silent) | ✓ |

If a new method is added to the real class, the bridge `.cpp` will fail to link (missing
symbol) — a hard compile-time safety net. The `*Mock.h` must be kept in sync manually.

---

## 📖 API Reference

### `library()`

Creates a library (static or shared) with automatic source discovery, compiler warnings, PCH, and unit tests.

```cmake
library(<name>
    [SHARED]
    [NO_INSTALL]
    [VERSION <version>]
    [PCH <path>]
    [COMPILE_OPTIONS <opt1> <opt2> ...]
    [PUBLIC_DEPENDENCIES <dep1> <dep2> ...]
    [PRIVATE_DEPENDENCIES <dep1> <dep2> ...]
    [TEST_DEPENDENCIES <dep1> <dep2> ...]
    [SUBDIRECTORIES <s1> [<s2> ...]]
    [SOURCES <src1> [<src2> ...]]
)
```

#### Arguments

| Argument | Type | Required | Description |
|----------|------|----------|-------------|
| `<name>` | positional | **Yes** | Name of the library target |
| `SHARED` | flag | No | Create a shared library (`.so`) instead of static (`.a`). Configures RPATH automatically. |
| `NO_INSTALL` | flag | No | Skip installation rules for this target. |
| `VERSION` | single value | No | Library version (sets `VERSION` and `SOVERSION` properties). |
| `PCH` | single value | No | Path to a custom PCH header (relative to module directory or absolute). Replaces the global PCH for this target. |
| `COMPILE_OPTIONS` | list | No | Extra compile flags for this target only (e.g., `-Wno-conversion` to suppress a warning). |
| `PUBLIC_DEPENDENCIES` | list | No | Libraries linked with `PUBLIC` visibility. Propagated to consumers of this library. |
| `PRIVATE_DEPENDENCIES` | list | No | Libraries linked with `PRIVATE` visibility. Used only internally. |
| `TEST_DEPENDENCIES` | list | No | Additional libraries linked to the `<name>_tests` test executable. |
| `SUBDIRECTORIES` | list | No | If set, restricts discovery to `include/<s>/`, `src/<s>/`, `tests/<s>/`, and `mocks/<s>/` for each name `s` in the list (still recursive under those roots). If omitted, the whole `include/`, `src/`, `tests/`, and `mocks/` trees are used. **Mutually exclusive with `SOURCES`.** |
| `SOURCES` | list | No | Explicit list of source files to compile (no globbing). Headers are still discovered automatically. **Mutually exclusive with `SUBDIRECTORIES`.** |

#### Automatic Behaviors

- **Source discovery**: By default, globs `src/**/*.cpp`, `src/**/*.c`, `include/**/*.hpp`, `include/**/*.h`, and the same extensions under `src/` for private headers, with `CONFIGURE_DEPENDS`. When `SOURCES` is specified, uses the explicit file list instead (headers are still discovered automatically).
- **Library type**: Creates a `STATIC` library by default, or `SHARED` if the flag is provided.
- **RPATH (shared only)**: Configures `$ORIGIN/../lib` RPATH so executables find the `.so` at runtime.
- **Alias target**: Creates `${PROJECT_NAME}::<name>` alias for safe usage in `target_link_libraries()`.
- **Include directories**: `include/` is `PUBLIC`, `src/` is `PRIVATE`.
- **Compiler warnings**: Applies strict warnings via `target_set_warnings()`.
- **GNU + Debug**: [`Compiler.cmake`](cmake/Compiler.cmake) adds `_GLIBCXX_ASSERTIONS` globally for **GCC** in the **Debug** configuration (extra libstdc++ run-time checks).
- **Sanitizers**: Applies ASAN/UBSAN/TSAN if enabled globally.
- **Coverage**: Instruments for coverage if `ENABLE_COVERAGE` is `ON`.
- **PCH**: Uses the global PCH unless `PCH` argument is provided.
- **Installation**: Installs to `lib/`, `include/` under component `devel` (unless `NO_INSTALL`).
- **Tests**: Creates `<name>_tests` executable if `BUILD_TESTING` is `ON` and matching test sources exist.
- **Install (headers)**: Installs all of `include/` by default; with `SUBDIRECTORIES`, installs each `include/<s>/` only.

#### Examples

```cmake
# Demo layout: one CMake folder, several libs under include/<lib>/, src/<lib>/ (see robot/CMakeLists.txt)
library(kinematics
    VERSION 0.1.0
    SUBDIRECTORIES kinematics
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
)

# Library with version and temporary warning suppression
library(database
    VERSION 1.2.0
    PCH
        pch/pch.hpp
    COMPILE_OPTIONS
        -Wno-conversion    # suppress for this module only
    PUBLIC_DEPENDENCIES
        nlohmann_json::nlohmann_json
    PRIVATE_DEPENDENCIES
        SQLite::SQLite3
    TEST_DEPENDENCIES
        testutils
)

# Shared library
library(mysharedlib
    SHARED
    PUBLIC_DEPENDENCIES SomeLib::SomeLib
)

# Explicit source list (no globbing) - mutually exclusive with SUBDIRECTORIES
library(mylib
    SOURCES
        src/file1.cpp
        src/file2.cpp
        src/utils/helper.cpp
    PUBLIC_DEPENDENCIES SomeLib::SomeLib
)
```

### `executable()`

Creates an executable module with automatic source discovery, compiler warnings, PCH, and optional unit tests.

```cmake
executable(<name>
    [NO_INSTALL]
    [VERSION <version>]
    [PCH <path>]
    [COMPILE_OPTIONS <opt1> <opt2> ...]
    [DEPENDENCIES <dep1> <dep2> ...]
    [TEST_DEPENDENCIES <dep1> <dep2> ...]
    [SUBDIRECTORIES <s1> [<s2> ...]]
    [SOURCES <src1> [<src2> ...]]
)
```

#### Arguments

| Argument | Type | Required | Description |
|----------|------|----------|-------------|
| `<name>` | positional | **Yes** | Name of the executable target |
| `NO_INSTALL` | flag | No | Skip installation rules and runtime dependency deployment. |
| `VERSION` | single value | No | Version metadata for the executable. |
| `PCH` | single value | No | Path to a custom PCH header (relative to module directory or absolute). Replaces the global PCH for this target. |
| `COMPILE_OPTIONS` | list | No | Extra compile flags for this target only (e.g., `-Wno-shadow`). |
| `DEPENDENCIES` | list | No | Libraries linked with `PRIVATE` visibility. |
| `TEST_DEPENDENCIES` | list | No | Libraries for tests (overrides `DEPENDENCIES` for mock injection). Falls back to `DEPENDENCIES` if not specified. |
| `SUBDIRECTORIES` | list | No | Same meaning as for `library()` for `include/`, `src/`, and `tests/`. **Mutually exclusive with `SOURCES`.** |
| `SOURCES` | list | No | Explicit list of source files to compile (no globbing). Headers are still discovered automatically. **Mutually exclusive with `SUBDIRECTORIES`.** |

#### Automatic Behaviors

- **Source discovery**: By default, globs `*.c`, `*.cpp`, `*.h`, `*.hpp` under `include/` and `src/` (either whole trees or per `SUBDIRECTORIES` entry). When `SOURCES` is specified, uses the explicit file list instead (headers are still discovered automatically).
- **Include directories**: Both `include/` and `src/` are `PRIVATE`.
- **Compiler warnings**: Applies strict warnings via `target_set_warnings()`.
- **GNU + Debug**: Same global `_GLIBCXX_ASSERTIONS` in **Debug** for **GCC** as for libraries (see [`Compiler.cmake`](cmake/Compiler.cmake)).
- **Sanitizers**: Applies ASAN/UBSAN/TSAN if enabled globally.
- **Coverage**: Instruments for coverage if `ENABLE_COVERAGE` is `ON`.
- **Stack traces**: Automatically links `Backward::Backward` for crash stack traces.
- **PCH**: Uses the global PCH unless `PCH` argument is provided.
- **Installation**: Installs to `bin/` under component `runtime` (unless `NO_INSTALL`). Automatically deploys runtime shared library dependencies to `lib/`.
- **Tests**: Creates `<name>_tests` if `BUILD_TESTING` is `ON` and test sources exist. The test executable recompiles all `src/` sources **except** `main.cpp` / `main.c`, allowing you to test application logic without the entry point.

#### Examples

```cmake
# Demo executable (see application/CMakeLists.txt)
executable(application
    VERSION 1.0.0
    PCH pch/pch.hpp
    DEPENDENCIES      robot_controller
    TEST_DEPENDENCIES robot_controller_mock
)

# Explicit source list (no globbing) - mutually exclusive with SUBDIRECTORIES
executable(mytool
    SOURCES
        src/main.cpp
        src/commands.cpp
    DEPENDENCIES SomeLib::SomeLib
)
```

### 🧩 Dependency Graph

On each configuration, CMake generates a dependency graph in Graphviz format at `build/dependencies.dot` (or `build/build/<Release|Debug>/dependencies.dot` depending on your build preset).

- **Green edges**: public dependencies
- **Red edges**: private dependencies
- **Blue nodes**: internal libraries
- **Cyan nodes**: executables
- **Grey nodes**: external libraries (typically from Conan)

To visualize the dependency graph, run:

```bash
dot -Tpng build/dependencies.dot -o dependencies.png
```

For example:

![dependencies](dependencies.png)

---

## 📝 Precompiled Headers (PCH)

### 🤔 When to Use Custom PCH

PCH significantly speeds up compilation by pre-processing commonly used headers once and reusing them across all translation units (i.e. STL, Qt, Boost ...).

| Situation | Recommendation |
|-----------|----------------|
| Standard module using only STL | Use global PCH (default) |
| Module with mp-units, Qt, Boost, or other heavy headers | Use custom PCH per module |
| Header-only module | No PCH needed |

### 🌐 Global PCH

By default, all targets use an auto-generated global PCH located at `${CMAKE_BINARY_DIR}/generated/global_pch.hpp`. This file contains commonly used STL headers:

- Containers: `<vector>`, `<map>`, `<set>`, `<unordered_map>`, `<unordered_set>`, `<array>`, `<deque>`, `<list>`
- Strings: `<string>`, `<string_view>`
- Memory: `<memory>`
- Utilities: `<algorithm>`, `<functional>`, `<optional>`, `<tuple>`, `<utility>`, `<variant>`
- I/O: `<iostream>`, `<sstream>`
- Misc: `<chrono>`, `<cstddef>`, `<cstdint>`, `<stdexcept>`, `<type_traits>`

The global PCH is compiled once and shared across all targets via CMake's `REUSE_FROM` mechanism.

### 🧰 Custom PCH

For modules that use heavy third-party headers (mp-units, Qt, Boost, etc.), you can specify a custom PCH:

```cmake
library(mymodule
    PCH pch/pch.hpp
)
```

**Important:** A custom PCH **completely replaces** the global PCH for that target. You can include the global PCH in your custom PCH to get all STL headers plus your module-specific headers:

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
