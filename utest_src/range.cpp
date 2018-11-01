#include "range.hpp"
#include <gtest/gtest.h>

TEST(range, non_overlap)
{
    RangeList ranges(10, 20);      // 10 - 20
    ranges.insertRange(5, 8);      // 5 - 8
    ranges.insertRange(100, 200);  // 100 - 200
    ranges.insertRange(21, 50);    // 21 - 50
    ASSERT_EQ(ranges.count(), 4);
    auto iter = ranges.begin();
    ASSERT_EQ(iter++->start, 5);
    ASSERT_EQ(iter++->start, 10);
    ASSERT_EQ(iter++->start, 21);
    ASSERT_EQ(iter++->start, 100);
}

TEST(range, overlap1)
{
    RangeList ranges(10, 20);      // 10 - 20
    ranges.insertRange(5, 8);      // 5 - 8
    ranges.insertRange(100, 200);  // 100 - 200
    ranges.insertRange(30, 50);    // 30 - 50
    ranges.insertRange(20, 30);    // 20 - 30
    ASSERT_EQ(ranges.count(), 3);
    auto iter = ranges.begin();
    ASSERT_EQ(iter++->start, 5);
    ASSERT_EQ(iter->start, 10);
    ASSERT_EQ(iter++->end, 50);
    ASSERT_EQ(iter++->start, 100);
}

TEST(range, overlap2)
{
    RangeList ranges(10, 20);      // 10 - 20
    ranges.insertRange(5, 8);      // 5 - 8
    ranges.insertRange(100, 200);  // 100 - 200
    ranges.insertRange(30, 50);    // 30 - 50
    ranges.insertRange(20, 30);    // 20 - 30
    ranges.insertRange(20, 120);   // 20 - 120
    ASSERT_EQ(ranges.count(), 2);
    auto iter = ranges.begin();
    ASSERT_EQ(iter++->start, 5);
    ASSERT_EQ(iter->start, 10);
    ASSERT_EQ(iter++->end, 200);
}

TEST(range, overlap3)
{
    RangeList ranges(10, 20);      // 10 - 20
    ranges.insertRange(5, 8);      // 5 - 8
    ranges.insertRange(100, 200);  // 100 - 200
    ranges.insertRange(30, 50);    // 30 - 50
    ranges.insertRange(1, 1000);   // 20 - 120
    ASSERT_EQ(ranges.count(), 1);
    auto iter = ranges.begin();
    ASSERT_EQ(iter->start, 1);
    ASSERT_EQ(iter++->end, 1000);
}

TEST(range, overlap4)
{
    RangeList ranges(10, 20);      // 10 - 20
    ranges.insertRange(5, 8);      // 5 - 8
    ranges.insertRange(100, 200);  // 100 - 200
    ranges.insertRange(30, 50);    // 30 - 50
    ranges.insertRange(200, 205);
    ASSERT_EQ(ranges.count(), 4);
    auto iter = ranges.begin();
    ASSERT_EQ(iter++->start, 5);
    ASSERT_EQ(iter++->start, 10);
    ASSERT_EQ(iter++->start, 30);
    ASSERT_EQ(iter->start, 100);
    ASSERT_EQ(iter->end, 205);
}
