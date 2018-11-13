#include "msg.hpp"
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

TEST(msg, serial_msg_open)
{
    MsgOpen msg(123, 456, "ece590", 12);
    const std::string tmpfile = "ece590-msg-serial";
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
        ASSERT_EQ(ptr->mode, msg.mode);
    }
}

TEST(msg, serial_msg_open_resp)
{
    MsgOpenResp msg(234, -10, -1000, -2000);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgOpenResp*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
        ASSERT_EQ(ptr->mtime_sec, msg.mtime_sec);
        ASSERT_EQ(ptr->mtime_nsec, msg.mtime_nsec);
    }
}

TEST(msg, serial_msg_stat)
{
    MsgStat msg(234, "myfile.cpp");
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgStat*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}

TEST(msg, serial_msg_stat_resp)
{
    MsgStatResp msg(234, 1,
                    {1000, 123, -1000, -2000, -3000, -4000, -5000, -6000});
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgStatResp*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
        ASSERT_EQ(ptr->stat.size, msg.stat.size);
        ASSERT_EQ(ptr->stat.mode, msg.stat.mode);
        ASSERT_EQ(ptr->stat.atime_sec, msg.stat.atime_sec);
        ASSERT_EQ(ptr->stat.atime_nsec, msg.stat.atime_nsec);
        ASSERT_EQ(ptr->stat.mtime_sec, msg.stat.mtime_sec);
        ASSERT_EQ(ptr->stat.mtime_nsec, msg.stat.mtime_nsec);
        ASSERT_EQ(ptr->stat.ctime_sec, msg.stat.ctime_sec);
        ASSERT_EQ(ptr->stat.ctime_nsec, msg.stat.ctime_nsec);
    }
}

TEST(msg, serial_msg_readdir)
{
    MsgReaddir msg(100, "./readdir");
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgReaddir*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}

TEST(msg, serial_msg_readdir_resp)
{
    MsgReaddirResp msg(100, -2, {"file1", "file2", "dir1"});
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgReaddirResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
        ASSERT_EQ(ptr->dir_names, msg.dir_names);
    }
}

TEST(msg, serial_msg_read)
{
    MsgRead msg(100, {"somefile"}, 10, 1000);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgRead*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
        ASSERT_EQ(ptr->offset, msg.offset);
        ASSERT_EQ(ptr->size, msg.size);
    }
}

TEST(msg, serial_msg_read_resp)
{
    MsgReadResp msg(1, 2, {'t', 'h', 'i', 's', ' ', 'd', 'a', 't', 'a'});
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgReadResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
        ASSERT_EQ(ptr->data, msg.data);
    }
}

TEST(msg, serial_msg_write)
{
    MsgWrite msg(1, "file1.cpp", 5, {'d', 'a', 't', 'a'});
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgWrite*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
        ASSERT_EQ(ptr->offset, msg.offset);
        ASSERT_EQ(ptr->data, msg.data);
    }
}

TEST(msg, serial_msg_write_resp)
{
    MsgWriteResp msg(1, 5);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgWriteResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
    }
}

TEST(msg, serial_msg_truncate)
{
    MsgTruncate msg(1, "file1", 1024);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgTruncate*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
        ASSERT_EQ(ptr->offset, msg.offset);
    }
}

TEST(msg, serial_msg_truncate_resp)
{
    MsgTruncateResp msg(1, 123);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgTruncateResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
    }
}

TEST(msg, serial_msg_unlink)
{
    MsgUnlink msg(1, "file1");
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgUnlink*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}

TEST(msg, serial_msg_unlink_resp)
{
    MsgUnlinkResp msg(1, 123);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgUnlinkResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
    }
}

TEST(msg, serial_msg_rmdir)
{
    MsgRmdir msg(1, "file1");
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgRmdir*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}

TEST(msg, serial_msg_rmdir_resp)
{
    MsgRmdirResp msg(1, 123);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgRmdirResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
    }
}

TEST(msg, serial_msg_mkdir)
{
    MsgMkdir msg(1, "dir1", 2);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgMkdir*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
        ASSERT_EQ(ptr->mode, msg.mode);
    }
}

TEST(msg, serial_msg_mkdir_resp)
{
    MsgMkdirResp msg(1, 123);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgMkdirResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
    }
}
