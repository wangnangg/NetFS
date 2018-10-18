#include "msg.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "stream.hpp"

static SReader tmpReader(const std::string& fname)
{
    int fd = open(("/tmp/" + fname).c_str(), O_RDONLY);
    auto fdrd = FdReader(fd);
    return [fdrd = std::move(fdrd)](char* buf, size_t size) mutable {
        fdrd.read(buf, size);
    };
}

static SWriter tmpWriter(const std::string& fname)
{
    int fd = open(("/tmp/" + fname).c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRWXU | S_IRWXG | S_IRWXO);
    auto wt = FdWriter(fd);
    return [wt = std::move(wt)](const char* buf, size_t size) mutable {
        wt.write(buf, size);
    };
}

TEST(msg, serial_msg_open)
{
    MsgOpen msg(123, 456, "ece590");
    const std::string tmpfile = "ece590-msg-serial_msg_open";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgOpen*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->flag, msg.flag);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}
