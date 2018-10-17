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
