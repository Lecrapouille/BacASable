#include <gtest/gtest.h>
#include "Set.hpp"
#include <string>
#include <vector>

using namespace container;

// ============================================================================
// Construction Tests
// ============================================================================

TEST(SetTest, DefaultConstruction)
{
    Set<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0u);
    EXPECT_FALSE(static_cast<bool>(set));
}

TEST(SetTest, ConstructionWithReservation)
{
    Set<int, 4> set(32);  // Reserve space for 32 elements (2 blocks of 16)
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0u);
    EXPECT_GE(set.capacity(), 32u);
}

TEST(SetTest, InitializerListConstruction)
{
    Set<int> set = {1, 2, 3, 4, 5};
    EXPECT_EQ(set.size(), 5u);
    EXPECT_EQ(set[0], 1);
    EXPECT_EQ(set[1], 2);
    EXPECT_EQ(set[2], 3);
    EXPECT_EQ(set[3], 4);
    EXPECT_EQ(set[4], 5);
}

// ============================================================================
// Element Access Tests
// ============================================================================

TEST(SetTest, OperatorBracketAccess)
{
    Set<int> set = {10, 20, 30};
    EXPECT_EQ(set[0], 10);
    EXPECT_EQ(set[1], 20);
    EXPECT_EQ(set[2], 30);

    // Modify through operator[]
    set[1] = 200;
    EXPECT_EQ(set[1], 200);
}

TEST(SetTest, AtAccessWithBoundsCheck)
{
    Set<int> set = {10, 20, 30};
    EXPECT_EQ(set.at(0), 10);
    EXPECT_EQ(set.at(1), 20);
    EXPECT_EQ(set.at(2), 30);

    // Modify through at()
    set.at(1) = 200;
    EXPECT_EQ(set.at(1), 200);
}

TEST(SetTest, AtThrowsOnOutOfBounds)
{
    Set<int> set = {1, 2, 3};
    EXPECT_THROW((void)set.at(3), std::out_of_range);
    EXPECT_THROW((void)set.at(100), std::out_of_range);

    Set<int> empty_set;
    EXPECT_THROW((void)empty_set.at(0), std::out_of_range);
}

TEST(SetTest, ConstAccess)
{
    const Set<int> set = {1, 2, 3};
    EXPECT_EQ(set[0], 1);
    EXPECT_EQ(set.at(1), 2);
}

// ============================================================================
// Modifier Tests
// ============================================================================

TEST(SetTest, AppendSingleElement)
{
    Set<int> set;
    set += 42;
    EXPECT_EQ(set.size(), 1u);
    EXPECT_EQ(set[0], 42);

    set += 100;
    EXPECT_EQ(set.size(), 2u);
    EXPECT_EQ(set[1], 100);
}

TEST(SetTest, AppendMultipleElements)
{
    Set<int> set;
    set += {1, 2, 3, 4, 5};
    EXPECT_EQ(set.size(), 5u);
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(set[i], static_cast<int>(i + 1));
    }
}

TEST(SetTest, AppendFromSpan)
{
    Set<int> set;
    std::vector<int> data = {10, 20, 30};
    set += std::span<const int>(data);
    EXPECT_EQ(set.size(), 3u);
    EXPECT_EQ(set[0], 10);
    EXPECT_EQ(set[1], 20);
    EXPECT_EQ(set[2], 30);
}

TEST(SetTest, AppendManyElements)
{
    Set<int, 3> set;  // Blocks of 8 elements

    // Add 100 elements (will require multiple blocks)
    for (int i = 0; i < 100; ++i)
    {
        set += i;
    }

    EXPECT_EQ(set.size(), 100u);
    for (size_t i = 0u; i < 100u; ++i)
    {
        EXPECT_EQ(set[i], static_cast<int>(i));
    }
}

TEST(SetTest, RemoveLastElement)
{
    Set<int> set = {1, 2, 3};
    set.pop_back();
    EXPECT_EQ(set.size(), 2u);
    EXPECT_EQ(set[0], 1);
    EXPECT_EQ(set[1], 2);
}

TEST(SetTest, RemoveMiddleElement)
{
    Set<int> set = {1, 2, 3, 4, 5};
    set.remove(2);  // Remove element at index 2 (value 3)

    // Should have swapped with last element (5)
    EXPECT_EQ(set.size(), 4u);
    EXPECT_EQ(set[0], 1);
    EXPECT_EQ(set[1], 2);
    EXPECT_EQ(set[2], 5);  // Was swapped with last
    EXPECT_EQ(set[3], 4);
}

TEST(SetTest, RemoveFirstElement)
{
    Set<int> set = {10, 20, 30};
    set.remove(0);
    EXPECT_EQ(set.size(), 2u);
    EXPECT_EQ(set[0], 30);  // Swapped with last
    EXPECT_EQ(set[1], 20);
}

TEST(SetTest, PopBackOnEmpty)
{
    Set<int> set;
    set.pop_back();  // Should not crash
    EXPECT_TRUE(set.empty());
}

TEST(SetTest, SwapElements)
{
    Set<int> set = {1, 2, 3, 4, 5};
    set.swap(1, 3);
    EXPECT_EQ(set[1], 4);
    EXPECT_EQ(set[3], 2);
}

TEST(SetTest, SwapSameIndex)
{
    Set<int> set = {1, 2, 3};
    set.swap(1, 1);  // Should do nothing
    EXPECT_EQ(set[1], 2);
}

TEST(SetTest, Clear)
{
    Set<int> set = {1, 2, 3, 4, 5};
    set.clear();
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0u);

    // Should be able to add elements again
    set += 42;
    EXPECT_EQ(set.size(), 1u);
    EXPECT_EQ(set[0], 42);
}

// ============================================================================
// Capacity Tests
// ============================================================================

TEST(SetTest, SizeAndEmpty)
{
    Set<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0u);

    set += 1;
    EXPECT_FALSE(set.empty());
    EXPECT_EQ(set.size(), 1u);

    set += {2, 3, 4};
    EXPECT_EQ(set.size(), 4u);
}

TEST(SetTest, BoolConversion)
{
    Set<int> set;
    EXPECT_FALSE(static_cast<bool>(set));

    set += 1;
    EXPECT_TRUE(static_cast<bool>(set));

    if (set)
    {
        SUCCEED();
    }
    else
    {
        FAIL() << "Bool conversion failed";
    }
}

TEST(SetTest, ShrinkToFit)
{
    Set<int, 3> set;  // Blocks of 8 elements

    // Add elements to create multiple blocks
    for (int i = 0; i < 24; ++i)
    {
        set += i;
    }
    EXPECT_GE(set.block_count(), 3u);

    // Remove all but first 8
    while (set.size() > 8)
    {
        set.pop_back();
    }

    // Shrink should remove empty blocks
    set.shrink_to_fit();
    EXPECT_EQ(set.size(), 8u);
}

// ============================================================================
// Iterator Tests
// ============================================================================

TEST(SetTest, RangeBasedFor)
{
    Set<int> set = {1, 2, 3, 4, 5};

    int sum = 0;
    for (const auto& elem : set)
    {
        sum += elem;
    }
    EXPECT_EQ(sum, 15);
}

TEST(SetTest, ModifyThroughIterator)
{
    Set<int> set = {1, 2, 3};

    for (auto& elem : set)
    {
        elem *= 2;
    }

    EXPECT_EQ(set[0], 2);
    EXPECT_EQ(set[1], 4);
    EXPECT_EQ(set[2], 6);
}

TEST(SetTest, IteratorOnEmpty)
{
    Set<int> set;

    int count = 0;
    for ([[maybe_unused]] const auto& elem : set)
    {
        ++count;
    }
    EXPECT_EQ(count, 0);
    EXPECT_EQ(set.begin(), set.end());
}

TEST(SetTest, ConstIterator)
{
    const Set<int> set = {1, 2, 3};

    std::vector<int> values;
    for (const auto& elem : set)
    {
        values.push_back(elem);
    }

    EXPECT_EQ(values.size(), 3u);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
}

// ============================================================================
// Type Tests
// ============================================================================

TEST(SetTest, StringElements)
{
    Set<std::string> set;
    set += "hello";
    set += "world";

    EXPECT_EQ(set.size(), 2u);
    EXPECT_EQ(set[0], "hello");
    EXPECT_EQ(set[1], "world");
}

TEST(SetTest, StructElements)
{
    struct Point
    {
        int x = 0;
        int y = 0;
    };

    Set<Point> set;
    set += Point{1, 2};
    set += Point{3, 4};

    EXPECT_EQ(set.size(), 2u);
    EXPECT_EQ(set[0].x, 1);
    EXPECT_EQ(set[0].y, 2);
    EXPECT_EQ(set[1].x, 3);
    EXPECT_EQ(set[1].y, 4);
}

// ============================================================================
// Block Size Tests
// ============================================================================

TEST(SetTest, DifferentBlockSizes)
{
    Set<int, 2> small_blocks;   // 4 elements per block
    Set<int, 5> large_blocks;   // 32 elements per block

    for (int i = 0; i < 50; ++i)
    {
        small_blocks += i;
        large_blocks += i;
    }

    EXPECT_EQ(small_blocks.size(), 50u);
    EXPECT_EQ(large_blocks.size(), 50u);

    // Small blocks should have more blocks
    EXPECT_GT(small_blocks.block_count(), large_blocks.block_count());

    // Values should be the same
    for (size_t i = 0u; i < 50u; ++i)
    {
        EXPECT_EQ(small_blocks[i], static_cast<int>(i));
        EXPECT_EQ(large_blocks[i], static_cast<int>(i));
    }
}

// ============================================================================
// PendingData Tests
// ============================================================================

TEST(SetTest, PendingDataAfterInsert)
{
    Set<int, 3> set;  // 8 elements per block

    EXPECT_FALSE(set.has_pending_data());

    set += {1, 2, 3, 4, 5};

    EXPECT_TRUE(set.has_pending_data());
    auto [start, end] = set.get_pending_range();
    EXPECT_EQ(start, 0u);
    EXPECT_EQ(end, 5u);
}

TEST(SetTest, PendingDataClear)
{
    Set<int> set = {1, 2, 3};

    EXPECT_TRUE(set.has_pending_data());

    set.clear_pending();

    EXPECT_FALSE(set.has_pending_data());
}

TEST(SetTest, PendingDataAfterModification)
{
    Set<int> set = {1, 2, 3, 4, 5};
    set.clear_pending();  // Reset tracking

    EXPECT_FALSE(set.has_pending_data());

    // Modify element at index 2
    set[2] = 100;
    // Note: operator[] doesn't automatically track, need explicit tracking
    // But swap() does track
    set.swap(1, 3);

    EXPECT_TRUE(set.has_pending_data());
    auto [start, end] = set.get_pending_range();
    // Should include indices 1 and 3
    EXPECT_LE(start, 1u);
    EXPECT_GE(end, 4u);
}

TEST(SetTest, PendingDataMultipleBlocks)
{
    Set<int, 2> set;  // 4 elements per block

    // Add 12 elements (3 blocks)
    for (int i = 0; i < 12; ++i)
    {
        set += i;
    }

    EXPECT_TRUE(set.has_pending_data());
    auto [start, end] = set.get_pending_range();
    EXPECT_EQ(start, 0u);
    EXPECT_EQ(end, 12u);

    set.clear_pending();
    EXPECT_FALSE(set.has_pending_data());
}

TEST(SetTest, PendingDataWorkflow)
{
    // Simulate GPU sync workflow
    Set<float> vertices;

    // Initial data
    vertices += {1.0f, 2.0f, 3.0f};

    // Sync to GPU
    EXPECT_TRUE(vertices.has_pending_data());
    auto [s1, e1] = vertices.get_pending_range();
    EXPECT_EQ(s1, 0u);
    EXPECT_EQ(e1, 3u);
    // upload_to_gpu(&vertices[s1], e1 - s1);
    vertices.clear_pending();

    // No changes
    EXPECT_FALSE(vertices.has_pending_data());

    // Modify some vertices
    vertices.swap(0, 2);

    // Only modified range needs sync
    EXPECT_TRUE(vertices.has_pending_data());
    auto [s2, e2] = vertices.get_pending_range();
    // Range should cover indices 0 and 2
    EXPECT_EQ(s2, 0u);
    EXPECT_EQ(e2, 3u);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(SetTest, OutOfBoundsCheck)
{
    Set<int> set = {1, 2, 3};
    EXPECT_FALSE(set.out_of_bounds(0));
    EXPECT_FALSE(set.out_of_bounds(2));
    EXPECT_TRUE(set.out_of_bounds(3));
    EXPECT_TRUE(set.out_of_bounds(100));
}

TEST(SetTest, RemoveInvalidIndex)
{
    Set<int> set = {1, 2, 3};
    set.remove(100);  // Should not crash
    EXPECT_EQ(set.size(), 3u);
}

TEST(SetTest, SwapInvalidIndices)
{
    Set<int> set = {1, 2, 3};
    set.swap(0, 100);  // Should not crash
    set.swap(100, 0);  // Should not crash
    EXPECT_EQ(set[0], 1);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
