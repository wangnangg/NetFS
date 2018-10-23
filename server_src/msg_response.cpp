#include "msg_response.hpp"
#include <cassert>
#include <iostream>

std::unique_ptr<Msg> respondOpen(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgOpen*>(&msg);
    assert(ptr);
    std::cout << "MsgOpen id: " << ptr->id << ", filename: " << ptr->filename
              << ", flag: " << ptr->flag << ", mode: " << ptr->mode
              << std::endl;
    auto resp = std::make_unique<MsgOpenResp>();
    resp->id = ptr->id;
    resp->error = op.open(ptr->filename, ptr->flag, ptr->mode);
    return resp;
}

std::unique_ptr<Msg> respondStat(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgStat*>(&msg);
    assert(ptr);
    std::cout << "MsgStat id: " << ptr->id << ", filename: " << ptr->filename
              << std::endl;
    struct stat stbuf;
    auto resp = std::make_unique<MsgStatResp>();
    resp->id = ptr->id;
    resp->error = op.stat(ptr->filename, stbuf);
    resp->stat.size = stbuf.st_size;
    resp->stat.mode = stbuf.st_mode;
    return resp;
}

std::unique_ptr<Msg> respondReaddir(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgReaddir*>(&msg);
    assert(ptr);
    std::cout << "MsgReaddir id: " << ptr->id
              << ", dirname: " << ptr->filename << std::endl;
    auto resp = std::make_unique<MsgReaddirResp>();
    resp->id = ptr->id;
    resp->error = op.readdir(ptr->filename, resp->dir_names);
    return resp;
}

std::unique_ptr<Msg> respondRead(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgRead*>(&msg);
    assert(ptr);
    std::cout << "MsgRead id: " << ptr->id << ", filename" << ptr->filename
              << ", offset: " << ptr->offset << ", size: " << ptr->size
              << std::endl;
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
    std::cout << "MsgWrite id: " << ptr->id << ", filename" << ptr->filename
              << ", offset: " << ptr->offset << ", size: " << ptr->data.size()
              << std::endl;
    auto resp = std::make_unique<MsgWriteResp>();
    resp->id = ptr->id;
    assert(ptr->data.size() > 0);
    resp->error =
        op.write(ptr->filename, ptr->offset, &ptr->data[0], ptr->data.size());
    return resp;
}

std::unique_ptr<Msg> respondTruncate(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgTruncate*>(&msg);
    assert(ptr);
    std::cout << "MsgTruncate id: " << ptr->id
              << ", filename: " << ptr->filename
              << ", offset: " << ptr->offset << std::endl;
    auto resp = std::make_unique<MsgTruncateResp>();
    resp->id = ptr->id;
    resp->error = op.truncate(ptr->filename, ptr->offset);
    return resp;
}

std::unordered_map<Msg::Type,
                   std::unique_ptr<Msg> (*)(const Msg& msg, FileOp& op)>
    responder_map = {
        {Msg::Open, respondOpen},       {Msg::Stat, respondStat},
        {Msg::Readdir, respondReaddir}, {Msg::Read, respondRead},
        {Msg::Write, respondWrite},     {Msg::Truncate, respondTruncate},
};
