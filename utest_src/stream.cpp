#include "stream.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>

TEST(stream, read_smoke)
{
    int fd = open("/dev/zero", O_RDONLY);
    assert(fd > 0);
    char buf[] = "0123456789";
    auto rdr = FdReader(fd);
    auto size = rdr.read(buf, 10);
    ASSERT_EQ(size, 10);
    for (size_t i = 0; i < size; i++)
    {
        ASSERT_EQ(buf[0], 0);
    }
}

TEST(stream, write_smoke)
{
    int fd = open("/dev/null", O_WRONLY);
    assert(fd > 0);
    char buf[] = "0123456789";
    auto wr = FdWriter(fd);
    auto size = wr.write(buf, 10);
    ASSERT_EQ(size, 10);
}

TEST(stream, read_write)
{
    {
        int fd = open("/tmp/ece590-utset-read-write", O_WRONLY | O_CREAT,
                      S_IRWXU | S_IRWXG | S_IRWXO);
        ASSERT_GT(fd, 0);
        char buf[] = "0123456789";
        auto wr = FdWriter(fd);
        auto size = wr.write(buf, 10);
        ASSERT_EQ(size, 10);
    }
    {
        int fd = open("/tmp/ece590-utset-read-write", O_RDONLY);
        ASSERT_GT(fd, 0);
        char buf[10];
        auto rd = FdReader(fd);
        auto size = rd.read(buf, 10);
        ASSERT_EQ(size, 10);
        for (int i = 0; i < 10; i++)
        {
            ASSERT_EQ(buf[i], '0' + i);
        }
    }
}
