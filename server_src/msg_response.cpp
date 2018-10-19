#include "msg_response.hpp"
#include <cassert>
#include <iostream>

std::unique_ptr<Msg> respondOpen(const Msg& msg, FileOp& op)
{
    auto ptr = dynamic_cast<const MsgOpen*>(&msg);
    assert(ptr);
    std::cout << "MsgOpen id: " << ptr->id << ", filename: " << ptr->filename
              << ", flag: " << ptr->flag << std::endl;
    auto resp = std::make_unique<MsgOpenResp>();
    resp->id = ptr->id;
    resp->error = op.open(ptr->filename, ptr->flag);
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

std::unordered_map<Msg::Type,
                   std::unique_ptr<Msg> (*)(const Msg& msg, FileOp& op)>
    responder_map = {
        {Msg::Open, respondOpen},
        {Msg::Stat, respondStat},
        {Msg::Readdir, respondReaddir},
};
