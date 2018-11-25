#include "msg_response.hpp"
#include <cassert>
#include <iostream>

std::unique_ptr<Msg> respondAccess(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgAccess*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgAccess id: " << ptr->id
              << ", filename: " << ptr->filename << std::endl;
#endif
    auto resp = std::make_unique<MsgAccessResp>();
    resp->id = ptr->id;
    resp->error = op.access(ptr->filename, resp->time);
    return resp;
}

std::unique_ptr<Msg> respondCreate(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgCreate*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgCreate id: " << ptr->id
              << ", filename: " << ptr->filename << std::endl;
#endif
    auto resp = std::make_unique<MsgCreateResp>();
    resp->id = ptr->id;
    resp->error = op.creat(ptr->filename);
    return resp;
}

std::unique_ptr<Msg> respondStat(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgStat*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgStat id: " << ptr->id << ", filename: " << ptr->filename
              << std::endl;
#endif
    struct stat stbuf;
    auto resp = std::make_unique<MsgStatResp>();
    resp->id = ptr->id;
    resp->error = op.stat(ptr->filename, stbuf);
    resp->stat.size = stbuf.st_size;
    resp->stat.mode = stbuf.st_mode;
    resp->stat.time = makeFileTime(stbuf);
    return resp;
}

std::unique_ptr<Msg> respondStatfs(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgStatfs*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgStatfs id: " << ptr->id << std::endl;
#endif
    auto resp = std::make_unique<MsgStatfsResp>();
    resp->id = ptr->id;
    resp->error = op.statfs(resp->stat);
    return resp;
}

std::unique_ptr<Msg> respondReaddir(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgReaddir*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgReaddir id: " << ptr->id
              << ", dirname: " << ptr->filename << std::endl;
#endif
    auto resp = std::make_unique<MsgReaddirResp>();
    resp->id = ptr->id;
    resp->error = op.readdir(ptr->filename, resp->dir_names);
    return resp;
}

std::unique_ptr<Msg> respondRead(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgRead*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgRead id: " << ptr->id << ", filename" << ptr->filename
              << ", offset: " << ptr->offset << ", size: " << ptr->size
              << std::endl;
#endif
    auto resp = std::make_unique<MsgReadResp>();
    resp->id = ptr->id;
    resp->data = std::vector<char>(ptr->size);
    size_t read_size = 0;
    resp->error = op.read(ptr->filename, ptr->offset, ptr->size,
                          &resp->data[0], read_size);
    resp->data.resize(read_size);
    return resp;
}

std::unique_ptr<Msg> respondWrite(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgWrite*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgWrite id: " << ptr->id << ", filename" << ptr->filename
              << ", offset: " << ptr->offset << ", size: " << ptr->data.size()
              << std::endl;
#endif
    auto resp = std::make_unique<MsgWriteResp>();
    resp->id = ptr->id;
    assert(ptr->data.size() > 0);
    resp->error =
        op.write(ptr->filename, ptr->offset, &ptr->data[0], ptr->data.size(),
                 resp->before_change, resp->after_change);
    return resp;
}

std::unique_ptr<Msg> respondTruncate(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgTruncate*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgTruncate id: " << ptr->id
              << ", filename: " << ptr->filename
              << ", offset: " << ptr->offset << std::endl;
#endif
    auto resp = std::make_unique<MsgTruncateResp>();

    resp->id = ptr->id;
    resp->error = op.truncate(ptr->filename, ptr->offset, resp->before_change,
                              resp->after_change);
    return resp;
}

std::unique_ptr<Msg> respondUnlink(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgUnlink*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgUnlink id: " << ptr->id
              << ", filename: " << ptr->filename << std::endl;
#endif
    auto resp = std::make_unique<MsgUnlinkResp>();
    resp->id = ptr->id;
    resp->error = op.unlink(ptr->filename);
    return resp;
}

std::unique_ptr<Msg> respondRmdir(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgRmdir*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgRmdir id: " << ptr->id << ", filename: " << ptr->filename
              << std::endl;
#endif
    auto resp = std::make_unique<MsgRmdirResp>();
    resp->id = ptr->id;
    resp->error = op.rmdir(ptr->filename);
    return resp;
}

std::unique_ptr<Msg> respondMkdir(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgMkdir*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgMkdir id: " << ptr->id << ", filename: " << ptr->filename
              << std::endl;
#endif
    auto resp = std::make_unique<MsgMkdirResp>();
    resp->id = ptr->id;
    resp->error = op.mkdir(ptr->filename, ptr->mode);
    return resp;
}

std::unique_ptr<Msg> respondRename(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgRename*>(&msg);
    assert(ptr);
#ifndef NDEBUG
    std::cout << "MsgRename id: " << ptr->id << ", from: " << ptr->from
              << ", to: " << ptr->to << std::endl;
#endif
    auto resp = std::make_unique<MsgRenameResp>();
    resp->id = ptr->id;
    resp->error = op.rename(ptr->from, ptr->to, ptr->flags);
    return resp;
}
std::unordered_map<Msg::Type,
                   std::unique_ptr<Msg> (*)(const Msg& msg, FileOp& op)>
    responder_map = {
        {Msg::Access, respondAccess},   {Msg::Create, respondCreate},
        {Msg::Statfs, respondStatfs},   {Msg::Stat, respondStat},
        {Msg::Readdir, respondReaddir}, {Msg::Read, respondRead},
        {Msg::Write, respondWrite},     {Msg::Truncate, respondTruncate},
        {Msg::Unlink, respondUnlink},   {Msg::Rmdir, respondRmdir},
        {Msg::Mkdir, respondMkdir},     {Msg::Rename, respondRename}};
