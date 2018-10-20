#include "serial.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "stream.hpp"

static const char* getUserName()
{
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw)
    {
        return pw->pw_name;
    }

    return "";
}

static std::string tmpFilename(const std::string& name)
{
    return (std::string("/tmp/") + getUserName() + "." + name).c_str();
}

static SReader tmpReader(const std::string& fname)
{
    int fd = open(tmpFilename(fname).c_str(), O_RDONLY);
    auto fdrd = FdReader(fd);
    return [fdrd = std::move(fdrd)](char* buf, size_t size) mutable {
        fdrd.read(buf, size);
    };
}

static SWriter tmpWriter(const std::string& fname)
{
    int fd = open(tmpFilename(fname).c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRWXU | S_IRWXG | S_IRWXO);
    auto wt = FdWriter(fd);
    return [wt = std::move(wt)](const char* buf, size_t size) mutable {
        wt.write(buf, size);
    };
}

TEST(serial, basic_types)
{
    const std::string tmpfile = "ece590-serial-basic_types";
    {
        int32_t x = -42;
        auto ws = tmpWriter(tmpfile);
        serializePod<int32_t>(x, ws);
        x = 43;
        serializePod<uint32_t>((uint32_t)x, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        int32_t y = unserializePod<int32_t>(rs);
        ASSERT_EQ(y, -42);
        uint32_t y2 = unserializePod<uint32_t>(rs);
        ASSERT_EQ(y2, 43);
    }
}

TEST(serial, string)
{
    std::string str = "Hello world!";
    const std::string tmpfile = "ece590-serial-string";
    {
        auto ws = tmpWriter(tmpfile);
        serializeString(str, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeString(rs);
        ASSERT_EQ(res, str);
    }
}

TEST(serial, vector)
{
    std::vector<std::string> vec = {"str1", "string2", "str3"};
    const std::string tmpfile = "ece590-serial-vector";
    {
        auto ws = tmpWriter(tmpfile);
        serializeVector<std::string>(vec, serializeString, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeVector<std::string>(unserializeString, rs);
        ASSERT_EQ(res, vec);
    }
}

TEST(serial, vector_char)
{
    std::vector<char> vec = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
    const std::string tmpfile = "ece590-serial-vector";
    {
        auto ws = tmpWriter(tmpfile);
        serializeVectorChar(vec, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeVectorChar(rs);
        ASSERT_EQ(res, vec);
    }
}
