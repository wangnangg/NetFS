#include "cache.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

TEST(cache, put_take)
{
    Cache cache(5);
    cache.putBlock(0, {1, 2, 3, 4, 5});
    cache.putBlock(4, {5, 4, 3, 2, 1});
    ASSERT_EQ(cache.getBlock(0), std::vector<char>({1, 2, 3, 4, 5}));
    ASSERT_EQ(cache.getBlock(4), std::vector<char>({5, 4, 3, 2, 1}));
    ASSERT_FALSE(cache.blockInCache(3));
    ASSERT_TRUE(cache.blockInCache(0));
}

TEST(cache, overlay1)
{
    Cache cache(5);
    std::vector<char> data = {1, 2, 3, 4, 5};
    cache.writeToBlock(0, 0, &data[0], 2);
    cache.writeToBlock(0, 2, &data[2], 2);
    ASSERT_FALSE(cache.blockInCache(0));
    cache.writeToBlock(0, 4, &data[4], 1);
    ASSERT_TRUE(cache.blockInCache(0));
    ASSERT_EQ(cache.getBlock(0), std::vector<char>({1, 2, 3, 4, 5}));
}

TEST(cache, overlay2)
{
    Cache cache(5);
    std::vector<char> data1 = {1, 2, 3, 4, 5};
    std::vector<char> data2 = {9, 9, 9, 9, 9};
    cache.writeToBlock(0, 0, &data2[0], 4);
    cache.writeToBlock(0, 3, &data1[0], 2);
    ASSERT_TRUE(cache.blockInCache(0));
    ASSERT_EQ(cache.getBlock(0), std::vector<char>({9, 9, 9, 1, 2}));
}

TEST(cache, overlay3)
{
    Cache cache(5);
    std::vector<char> data1 = {1, 2, 3, 4, 5};
    std::vector<char> data2 = {9, 9, 9, 9, 9};
    cache.writeToBlock(0, 3, &data1[0], 2);
    ASSERT_FALSE(cache.blockInCache(0));
    cache.putBlock(0, data2);
    ASSERT_TRUE(cache.blockInCache(0));
    ASSERT_EQ(cache.getBlock(0), std::vector<char>({9, 9, 9, 1, 2}));
}
