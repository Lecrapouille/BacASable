# Container Library

A modern C++20 header-only library providing block-allocated containers optimized for large datasets.

## Features

- **Block allocation**: Minimizes memory allocation overhead by allocating elements in fixed-size blocks
- **O(1) operations**: Constant-time access, insertion, and removal
- **Pending data tracking**: Tracks modified elements for efficient synchronization (e.g., CPU-to-GPU transfers)
- **Modern C++20**: Uses concepts, `constexpr`, `std::popcount`, `std::unique_ptr`, and more
- **Header-only**: Just include and use, no compilation needed

## Containers

### Set<T, N>

A dense container without holes, similar to `std::vector` but with block allocation.

**Key properties:**
- O(1) append (amortized)
- O(1) access by index
- O(1) removal (swaps with last element to avoid holes)
- No holes: indices may change after removal

```cpp
#include "Set.hpp"

using namespace container;

Set<int, 4> set;              // Blocks of 16 elements (2^4)

// Append elements
set += 42;                    // Single element
set += {1, 2, 3, 4, 5};       // Multiple elements

// Access elements
int x = set[0];               // No bounds check (fast)
int y = set.at(0);            // With bounds check (throws on error)

// Remove elements
set.remove(0);                // Swaps with last, then removes
set.pop_back();               // Remove last element

// Iterate
for (auto& elem : set) {
    std::cout << elem << "\n";
}

// Check state
if (set) {                    // operator bool
    std::cout << "Size: " << set.size() << "\n";
}
```

### Collection<T, N>

A sparse container allowing holes, useful when element indices must remain stable.

**Key properties:**
- O(1) insertion at any index
- O(1) access by index
- O(1) removal (leaves a hole)
- Holes allowed: indices remain stable after removal
- Iterator automatically skips holes

```cpp
#include "Collection.hpp"

using namespace container;

Collection<std::string, 4> col;

// Insert at specific positions
col.insert(0, "first");
col.insert(10, "tenth");      // Gap between 0 and 10

// Append at end
col += "next";

// Remove (creates hole)
col.remove(0);

// Check if slot is occupied
if (col.occupied(10)) {
    std::cout << col[10] << "\n";
}

// Safe access (throws on hole or out-of-bounds)
try {
    auto& val = col.at(0);    // Throws: this is now a hole
} catch (const std::out_of_range& e) {
    std::cerr << e.what() << "\n";
}

// Iterate (skips holes automatically)
for (const auto& elem : col) {
    std::cout << elem << "\n";
}
```

## Block Size

The template parameter `N` controls the block size: each block holds `2^N` elements.

| N | Block Size | Good for |
|---|------------|----------|
| 2 | 4 elements | Small objects, testing |
| 4 | 16 elements | Default, balanced |
| 6 | 64 elements | Large datasets |
| 8 | 256 elements | Very large datasets |

```cpp
Set<int, 2> small;   // 4 elements per block
Set<int, 6> large;   // 64 elements per block
```

### Index Calculation

The global index is split into a **block index** and a **sub-index** within the block using bit operations:

```
block_index = global_index >> N        (equivalent to: global_index / 2^N)
sub_index   = global_index & (2^N - 1) (equivalent to: global_index % 2^N)
```

**Example with N=4 (blocks of 16 elements):**

| Global Index | Calculation | Block | Sub-index |
|--------------|-------------|-------|-----------|
| 0 | 0 >> 4 = 0, 0 & 15 = 0 | 0 | 0 |
| 15 | 15 >> 4 = 0, 15 & 15 = 15 | 0 | 15 |
| 16 | 16 >> 4 = 1, 16 & 15 = 0 | 1 | 0 |
| 17 | 17 >> 4 = 1, 17 & 15 = 1 | 1 | 1 |
| 31 | 31 >> 4 = 1, 31 & 15 = 15 | 1 | 15 |
| 32 | 32 >> 4 = 2, 32 & 15 = 0 | 2 | 0 |

**Example with N=7 (blocks of 128 elements):**

| Global Index | Calculation | Block | Sub-index |
|--------------|-------------|-------|-----------|
| 0 | 0 >> 7 = 0, 0 & 127 = 0 | 0 | 0 |
| 127 | 127 >> 7 = 0, 127 & 127 = 127 | 0 | 127 |
| 128 | 128 >> 7 = 1, 128 & 127 = 0 | 1 | 0 |
| 129 | 129 >> 7 = 1, 129 & 127 = 1 | 1 | 1 |
| 255 | 255 >> 7 = 1, 255 & 127 = 127 | 1 | 127 |
| 256 | 256 >> 7 = 2, 256 & 127 = 0 | 2 | 0 |

This approach provides O(1) index computation using fast bit operations instead of division/modulo.

```
Memory layout with N=4 (16 elements per block):

Block 0          Block 1          Block 2
[0..15]          [16..31]         [32..47]
+----+----+      +----+----+      +----+----+
| 0  | 1  |...   | 16 | 17 |...   | 32 | 33 |...
+----+----+      +----+----+      +----+----+
```

## Pending Data Tracking

Both containers track which elements have been modified since the last synchronization. This is useful for efficiently updating external resources (GPU buffers, databases, network sync, etc.).

```cpp
#include "Set.hpp"

using namespace container;

Set<float> vertices;

// Add initial data
vertices += {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};

// Check if data has been modified
if (vertices.has_pending_data()) {
    // Get the range of modified elements
    auto [start, end] = vertices.get_pending_range();

    // Sync only the modified portion (e.g., to GPU)
    // glBufferSubData(GL_ARRAY_BUFFER,
    //                 start * sizeof(float),
    //                 (end - start) * sizeof(float),
    //                 &vertices[start]);

    // Mark as synchronized
    vertices.clear_pending();
}

// Later modifications...
vertices.swap(0, 2);  // Automatically tracked

// Only sync modified data
if (vertices.has_pending_data()) {
    auto [start, end] = vertices.get_pending_range();
    // Efficient partial update instead of full buffer upload
    sync_partial(&vertices[start], end - start);
    vertices.clear_pending();
}
```

### PendingData API

| Method | Description |
|--------|-------------|
| `has_pending_data()` | Returns `true` if any elements have been modified |
| `get_pending_range()` | Returns `{start, end}` indices of the modified range |
| `clear_pending()` | Resets tracking (call after synchronization) |

### What Gets Tracked

- `operator+=` / insert operations
- `swap()` operations
- `remove()` operations (the swapped element)

**Note:** Direct access via `operator[]` does not automatically track modifications. Use `swap()` or re-insert the element if tracking is needed.

## Requirements

- C++20 compiler (GCC 10+, Clang 10+, MSVC 2019 16.8+)
- CMake 3.14+ (for building tests)

## Building and Testing

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build tests
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Or run directly
./build/test_set
./build/test_collection
```

## Installation

Just copy `include/Set.hpp` and `include/Collection.hpp` to your project.

## API Reference

### Set<T, N>

| Method | Description |
|--------|-------------|
| `operator[](index)` | Access element (no bounds check) |
| `at(index)` | Access element (throws on invalid index) |
| `operator+=(elem)` | Append single element |
| `operator+=(span)` | Append multiple elements |
| `remove(index)` | Remove element (swaps with last) |
| `pop_back()` | Remove last element |
| `swap(i, j)` | Swap two elements |
| `size()` | Number of elements |
| `empty()` | True if empty |
| `operator bool()` | True if not empty |
| `clear()` | Remove all elements |
| `shrink_to_fit()` | Release empty blocks |
| `begin()` / `end()` | Iterators |
| `has_pending_data()` | True if data has been modified |
| `get_pending_range()` | Range of modified indices {start, end} |
| `clear_pending()` | Reset modification tracking |

### Collection<T, N>

| Method | Description |
|--------|-------------|
| `operator[](index)` | Access element (no bounds check) |
| `at(index)` | Access element (throws on invalid index or hole) |
| `occupied(index)` | True if slot has an element |
| `operator+=(elem)` | Append at end |
| `insert(index, elem)` | Insert at specific position |
| `remove(index)` | Remove element (creates hole) |
| `size()` | Number of stored elements |
| `extent()` | Index after last element |
| `empty()` | True if empty |
| `operator bool()` | True if not empty |
| `clear()` | Remove all elements |
| `shrink_to_fit()` | Release empty blocks |
| `begin()` / `end()` | Iterators (skip holes) |
| `has_pending_data()` | True if data has been modified |
| `get_pending_range()` | Range of modified indices {start, end} |
| `clear_pending()` | Reset modification tracking |
