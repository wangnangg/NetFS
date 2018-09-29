#include <gtest/gtest.h>

TEST(example, test1)
{
    int x = 1;
    int y = 2;
    ASSERT_EQ(x + y, 3);
};

TEST(example, test2)
{
    int x = 1;
    int y = 2;
    ASSERT_GT(y, x);
};
