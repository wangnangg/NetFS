#include "msg.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

static FdReader tmpReader(const std::string& fname)
{
    int fd = open(("/tmp/" + fname).c_str(), O_RDONLY);
    return FdReader(fd);
}

static FdWriter tmpWriter(const std::string& fname)
{
    int fd = open(("/tmp/" + fname).c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRWXU | S_IRWXG | S_IRWXO);
    return FdWriter(fd);
}

TEST(msg, serial_basic_types)
{
    const std::string tmpfile = "ece590-msg-serial_basic_types";
    {
        int32_t x = -42;
        auto ws = tmpWriter(tmpfile);
        serialize_int32(x, ws);
        x = 43;
        serialize_uint32((uint32_t)x, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        int32_t y = unserialize_int32(rs);
        ASSERT_EQ(y, -42);
        uint32_t y2 = unserialize_uint32(rs);
        ASSERT_EQ(y2, 43);
    }
}

TEST(msg, serial_string)
{
    std::string str = "Hello world!";
    const std::string tmpfile = "ece590-msg-serial_string";
    {
        auto ws = tmpWriter(tmpfile);
        serialize_string(str, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserialize_string(rs);
        ASSERT_EQ(res, str);
    }
}

TEST(msg, serial_msg_open)
{
    MsgOpen msg(123, 456, "ece590");
    const std::string tmpfile = "ece590-msg-serial_msg_open";
    {
        auto ws = tmpWriter(tmpfile);
        serialize_msg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserialize_msg(rs);
        auto ptr = dynamic_cast<MsgOpen*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->flag, msg.flag);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}
