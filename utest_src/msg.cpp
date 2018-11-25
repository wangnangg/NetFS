#include "msg.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
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

TEST(msg, serial_msg_access)
{
    MsgAccess msg(123, "ece590");
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgAccess*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}

TEST(msg, serial_msg_access_resp)
{
    MsgAccessResp msg(234, -10, {{-1000, -2000}, {3000, 400}, {5, 6}});
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgAccessResp*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
        ASSERT_EQ(ptr->time, msg.time);
    }
}

TEST(msg, serial_msg_create)
{
    MsgCreate msg(123, "ece590");
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgCreate*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->filename, msg.filename);
    }
}

TEST(msg, serial_msg_create_resp)
{
    MsgCreateResp msg(123, 234);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgCreateResp*>(res.get());
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
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
    MsgStatResp msg(
        234, 1,
        {1000, 123, {{-1000, -2000}, {-3000, -4000}, {-5000, -6000}}});
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
        ASSERT_EQ(ptr->stat.time, msg.stat.time);
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
    MsgWriteResp msg(1, 5, {{1, 2}, {3, 4}, {5, 6}},
                     {{7, 8}, {9, 10}, {11, 12}});
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
        ASSERT_EQ(ptr->before_change, msg.before_change);
        ASSERT_EQ(ptr->after_change, msg.after_change);
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
    MsgTruncateResp msg(1, 123, {{1, 2}, {3, 4}, {5, 6}},
                        {{7, 8}, {9, 10}, {11, 12}});
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
        ASSERT_EQ(ptr->before_change, msg.before_change);
        ASSERT_EQ(ptr->after_change, msg.after_change);
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

TEST(msg, serial_msg_statfs)
{
    MsgStatfs msg(1);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgStatfs*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
    }
}

TEST(msg, serial_msg_statfs_resp)
{
    struct statvfs buf;
    int err = statvfs(".", &buf);
    ASSERT_EQ(err, 0);
    MsgStatfsResp msg(1, 1, makeFsStat(buf));
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgStatfsResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
        ASSERT_EQ(ptr->stat.bsize, msg.stat.bsize);
        ASSERT_EQ(ptr->stat.frsize, msg.stat.frsize);
        ASSERT_EQ(ptr->stat.blocks, msg.stat.blocks);
        ASSERT_EQ(ptr->stat.bfree, msg.stat.bfree);
        ASSERT_EQ(ptr->stat.bavail, msg.stat.bavail);
        ASSERT_EQ(ptr->stat.files, msg.stat.files);
        ASSERT_EQ(ptr->stat.ffree, msg.stat.ffree);
        ASSERT_EQ(ptr->stat.namemax, msg.stat.namemax);
    }
}

TEST(msg, serial_msg_rename)
{
    MsgRename msg(1, "from", "to", 123);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgRename*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->from, msg.from);
        ASSERT_EQ(ptr->to, msg.to);
        ASSERT_EQ(ptr->flags, msg.flags);
    }
}

TEST(msg, serial_msg_rename_resp)
{
    MsgRenameResp msg(1, 123);
    const std::string tmpfile = "ece590-msg-serial";
    {
        auto ws = tmpWriter(tmpfile);
        serializeMsg(msg, ws);
    }
    {
        auto rs = tmpReader(tmpfile);
        auto res = unserializeMsg(rs);
        auto ptr = dynamic_cast<MsgRenameResp*>(res.get());
        ASSERT_TRUE(ptr);
        ASSERT_EQ(ptr->id, msg.id);
        ASSERT_EQ(ptr->error, msg.error);
    }
}
