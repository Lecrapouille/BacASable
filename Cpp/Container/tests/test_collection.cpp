#include <gtest/gtest.h>
#include "Collection.hpp"
#include <string>
#include <vector>
#include <set>

using namespace container;

// ============================================================================
// Construction Tests
// ============================================================================

TEST(CollectionTest, DefaultConstruction)
{
    Collection<int> col;
    EXPECT_TRUE(col.empty());
    EXPECT_EQ(col.size(), 0u);
    EXPECT_FALSE(static_cast<bool>(col));
}

TEST(CollectionTest, ConstructionWithReservation)
{
    Collection<int, 4> col(32);
    EXPECT_TRUE(col.empty());
    EXPECT_EQ(col.size(), 0u);
    EXPECT_GE(col.capacity(), 32u);
}

TEST(CollectionTest, InitializerListConstruction)
{
    Collection<int> col = {1, 2, 3, 4, 5};
    EXPECT_EQ(col.size(), 5u);
    EXPECT_EQ(col[0], 1);
    EXPECT_EQ(col[1], 2);
    EXPECT_EQ(col[2], 3);
    EXPECT_EQ(col[3], 4);
    EXPECT_EQ(col[4], 5);
}

// ============================================================================
// Element Access Tests
// ============================================================================

TEST(CollectionTest, OperatorBracketAccess)
{
    Collection<int> col = {10, 20, 30};
    EXPECT_EQ(col[0], 10);
    EXPECT_EQ(col[1], 20);
    EXPECT_EQ(col[2], 30);

    // Modify through operator[]
    col[1] = 200;
    EXPECT_EQ(col[1], 200);
}

TEST(CollectionTest, AtAccessWithBoundsCheck)
{
    Collection<int> col = {10, 20, 30};
    EXPECT_EQ(col.at(0), 10);
    EXPECT_EQ(col.at(1), 20);
    EXPECT_EQ(col.at(2), 30);

    // Modify through at()
    col.at(1) = 200;
    EXPECT_EQ(col.at(1), 200);
}

TEST(CollectionTest, AtThrowsOnOutOfBounds)
{
    Collection<int> col = {1, 2, 3};
    EXPECT_THROW((void)col.at(100), std::out_of_range);

    Collection<int> empty_col;
    EXPECT_THROW((void)empty_col.at(0), std::out_of_range);
}

TEST(CollectionTest, AtThrowsOnHole)
{
    Collection<int> col = {1, 2, 3};
    col.remove(1);  // Create hole at index 1

    EXPECT_NO_THROW((void)col.at(0));
    EXPECT_THROW((void)col.at(1), std::out_of_range);  // Hole!
    EXPECT_NO_THROW((void)col.at(2));
}

TEST(CollectionTest, OccupiedCheck)
{
    Collection<int> col = {1, 2, 3};
    EXPECT_TRUE(col.occupied(0));
    EXPECT_TRUE(col.occupied(1));
    EXPECT_TRUE(col.occupied(2));
    EXPECT_FALSE(col.occupied(3));

    col.remove(1);
    EXPECT_TRUE(col.occupied(0));
    EXPECT_FALSE(col.occupied(1));  // Hole
    EXPECT_TRUE(col.occupied(2));
}

TEST(CollectionTest, ConstAccess)
{
    const Collection<int> col = {1, 2, 3};
    EXPECT_EQ(col[0], 1);
    EXPECT_EQ(col.at(1), 2);
    EXPECT_TRUE(col.occupied(0));
}

// ============================================================================
// Modifier Tests
// ============================================================================

TEST(CollectionTest, AppendSingleElement)
{
    Collection<int> col;
    col += 42;
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col[0], 42);

    col += 100;
    EXPECT_EQ(col.size(), 2u);
    EXPECT_EQ(col[1], 100);
}

TEST(CollectionTest, AppendMultipleElements)
{
    Collection<int> col;
    col += {1, 2, 3, 4, 5};
    EXPECT_EQ(col.size(), 5u);
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(col[i], static_cast<int>(i + 1));
    }
}

TEST(CollectionTest, InsertAtSpecificIndex)
{
    Collection<int> col;
    col.insert(5, 100);  // Insert at index 5

    EXPECT_EQ(col.size(), 1u);
    EXPECT_TRUE(col.occupied(5));
    EXPECT_EQ(col[5], 100);
    EXPECT_FALSE(col.occupied(0));
    EXPECT_FALSE(col.occupied(4));
}

TEST(CollectionTest, InsertWithGaps)
{
    Collection<int> col;
    col.insert(0, 10);
    col.insert(5, 50);
    col.insert(10, 100);

    EXPECT_EQ(col.size(), 3u);
    EXPECT_TRUE(col.occupied(0));
    EXPECT_FALSE(col.occupied(3));
    EXPECT_TRUE(col.occupied(5));
    EXPECT_FALSE(col.occupied(7));
    EXPECT_TRUE(col.occupied(10));
}

TEST(CollectionTest, RemoveCreatesHole)
{
    Collection<int> col = {1, 2, 3, 4, 5};
    col.remove(2);

    EXPECT_EQ(col.size(), 4u);
    EXPECT_TRUE(col.occupied(0));
    EXPECT_TRUE(col.occupied(1));
    EXPECT_FALSE(col.occupied(2));  // Hole!
    EXPECT_TRUE(col.occupied(3));
    EXPECT_TRUE(col.occupied(4));

    // Values unchanged (unlike Set)
    EXPECT_EQ(col[0], 1);
    EXPECT_EQ(col[1], 2);
    EXPECT_EQ(col[3], 4);
    EXPECT_EQ(col[4], 5);
}

TEST(CollectionTest, RemoveFirstElement)
{
    Collection<int> col = {10, 20, 30};
    col.remove(0);

    EXPECT_EQ(col.size(), 2u);
    EXPECT_FALSE(col.occupied(0));
    EXPECT_TRUE(col.occupied(1));
    EXPECT_TRUE(col.occupied(2));
}

TEST(CollectionTest, RemoveLastElement)
{
    Collection<int> col = {10, 20, 30};
    col.remove(2);

    EXPECT_EQ(col.size(), 2u);
    EXPECT_EQ(col.extent(), 2u);  // End shrinks when removing last
}

TEST(CollectionTest, RemoveAlreadyEmpty)
{
    Collection<int> col = {1, 2, 3};
    col.remove(1);
    col.remove(1);  // Already a hole, should not crash

    EXPECT_EQ(col.size(), 2u);
}

TEST(CollectionTest, Clear)
{
    Collection<int> col = {1, 2, 3, 4, 5};
    col.clear();
    EXPECT_TRUE(col.empty());
    EXPECT_EQ(col.size(), 0u);

    // Should be able to add elements again
    col += 42;
    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col[0], 42);
}

// ============================================================================
// Capacity Tests
// ============================================================================

TEST(CollectionTest, SizeCountsOnlyOccupied)
{
    Collection<int> col = {1, 2, 3, 4, 5};
    EXPECT_EQ(col.size(), 5u);

    col.remove(1);
    col.remove(3);
    EXPECT_EQ(col.size(), 3u);  // 2 holes

    // extent() should still be 5
    EXPECT_EQ(col.extent(), 5u);
}

TEST(CollectionTest, BoolConversion)
{
    Collection<int> col;
    EXPECT_FALSE(static_cast<bool>(col));

    col += 1;
    EXPECT_TRUE(static_cast<bool>(col));
}

TEST(CollectionTest, ExtentTracking)
{
    Collection<int> col;
    EXPECT_EQ(col.extent(), 0u);

    col.insert(10, 100);
    EXPECT_EQ(col.extent(), 11u);  // One past last element

    col.insert(5, 50);
    EXPECT_EQ(col.extent(), 11u);  // Still 11

    col.remove(10);
    EXPECT_EQ(col.extent(), 6u);  // Shrinks when removing last
}

// ============================================================================
// Iterator Tests (holes should be skipped)
// ============================================================================

TEST(CollectionTest, IteratorSkipsHoles)
{
    Collection<int> col = {1, 2, 3, 4, 5};
    col.remove(1);
    col.remove(3);

    std::vector<int> values;
    for (const auto& elem : col)
    {
        values.push_back(elem);
    }

    EXPECT_EQ(values.size(), 3u);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 3);  // Skipped hole at 1, so 3 is second
    EXPECT_EQ(values[2], 5);  // Skipped hole at 3, so 5 is third
}

TEST(CollectionTest, IteratorOnSparseCollection)
{
    Collection<int> col;
    col.insert(0, 10);
    col.insert(5, 50);
    col.insert(10, 100);

    std::vector<int> values;
    for (const auto& elem : col)
    {
        values.push_back(elem);
    }

    EXPECT_EQ(values.size(), 3u);
    EXPECT_EQ(values[0], 10);
    EXPECT_EQ(values[1], 50);
    EXPECT_EQ(values[2], 100);
}

TEST(CollectionTest, ModifyThroughIterator)
{
    Collection<int> col = {1, 2, 3};
    col.remove(1);  // Create hole

    for (auto& elem : col)
    {
        elem *= 10;
    }

    EXPECT_EQ(col[0], 10);
    EXPECT_EQ(col[2], 30);
}

TEST(CollectionTest, IteratorOnEmpty)
{
    Collection<int> col;

    int count = 0;
    for ([[maybe_unused]] const auto& elem : col)
    {
        ++count;
    }
    EXPECT_EQ(count, 0);
    EXPECT_EQ(col.begin(), col.end());
}

TEST(CollectionTest, ConstIterator)
{
    const Collection<int> col = {1, 2, 3};

    std::vector<int> values;
    for (const auto& elem : col)
    {
        values.push_back(elem);
    }

    EXPECT_EQ(values.size(), 3u);
}

TEST(CollectionTest, IteratorAfterRemovingAll)
{
    Collection<int> col = {1, 2, 3};
    col.remove(0);
    col.remove(1);
    col.remove(2);

    EXPECT_TRUE(col.empty());
    EXPECT_EQ(col.begin(), col.end());
}

// ============================================================================
// Sparse Usage Patterns
// ============================================================================

TEST(CollectionTest, SparseGraphStyleUsage)
{
    // Simulate graph node storage where IDs must remain stable
    Collection<std::string> nodes;

    nodes.insert(0, "A");
    nodes.insert(1, "B");
    nodes.insert(2, "C");
    nodes.insert(3, "D");

    // Remove node B, but C and D keep their indices
    nodes.remove(1);

    EXPECT_EQ(nodes[0], "A");
    EXPECT_FALSE(nodes.occupied(1));
    EXPECT_EQ(nodes[2], "C");
    EXPECT_EQ(nodes[3], "D");

    // Insert new node at the hole
    nodes.insert(1, "E");
    EXPECT_EQ(nodes[1], "E");
}

TEST(CollectionTest, ReinsertAtHole)
{
    Collection<int> col = {1, 2, 3};
    col.remove(1);

    EXPECT_FALSE(col.occupied(1));
    EXPECT_EQ(col.size(), 2u);

    col.insert(1, 200);

    EXPECT_TRUE(col.occupied(1));
    EXPECT_EQ(col[1], 200);
    EXPECT_EQ(col.size(), 3u);
}

// ============================================================================
// Type Tests
// ============================================================================

TEST(CollectionTest, StringElements)
{
    Collection<std::string> col;
    col += "hello";
    col += "world";

    EXPECT_EQ(col.size(), 2u);
    EXPECT_EQ(col[0], "hello");
    EXPECT_EQ(col[1], "world");
}

TEST(CollectionTest, StructElements)
{
    struct Point
    {
        int x = 0;
        int y = 0;
    };

    Collection<Point> col;
    col += Point{1, 2};
    col += Point{3, 4};

    EXPECT_EQ(col.size(), 2u);
    EXPECT_EQ(col[0].x, 1);
    EXPECT_EQ(col[0].y, 2);
}

// ============================================================================
// Block Size Tests
// ============================================================================

TEST(CollectionTest, DifferentBlockSizes)
{
    Collection<int, 2> small_blocks;   // 4 elements per block
    Collection<int, 5> large_blocks;   // 32 elements per block

    for (int i = 0; i < 50; ++i)
    {
        small_blocks += i;
        large_blocks += i;
    }

    EXPECT_EQ(small_blocks.size(), 50u);
    EXPECT_EQ(large_blocks.size(), 50u);

    // Values should be the same
    for (size_t i = 0u; i < 50u; ++i)
    {
        EXPECT_EQ(small_blocks[i], static_cast<int>(i));
        EXPECT_EQ(large_blocks[i], static_cast<int>(i));
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(CollectionTest, RemoveInvalidIndex)
{
    Collection<int> col = {1, 2, 3};
    col.remove(100);  // Should not crash
    EXPECT_EQ(col.size(), 3u);
}

TEST(CollectionTest, InsertOverwrite)
{
    Collection<int> col = {1, 2, 3};
    col.insert(1, 200);  // Overwrite existing

    EXPECT_EQ(col.size(), 3u);  // Same size
    EXPECT_EQ(col[1], 200);
}

TEST(CollectionTest, LargeGap)
{
    Collection<int> col;
    col.insert(1000, 42);

    EXPECT_EQ(col.size(), 1u);
    EXPECT_EQ(col[1000], 42);
    EXPECT_TRUE(col.occupied(1000));
    EXPECT_FALSE(col.occupied(0));
    EXPECT_FALSE(col.occupied(999));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
