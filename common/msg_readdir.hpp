#pragma once
#include "msg_base.hpp"

class MsgReaddir : public Msg
{
public:
    int32_t id;
    std::string filename;
    // more fields here
public:
    MsgReaddir() : Msg(Msg::Readdir), id(0), filename()  // more fields

    {
    }
    MsgReaddir(int32_t id, std::string filename)
        : Msg(Msg::Readdir),
          id(id),
          filename(std::move(filename))  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(filename, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgReaddir>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        return res;
    }
};

class MsgReaddirResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    std::vector<std::string> dir_names;
    // more fields here
public:
    MsgReaddirResp()
        : Msg(Msg::ReaddirResp), id(0), error(), dir_names()  // more fields

    {
    }
    MsgReaddirResp(int32_t id, int32_t error, std::vector<std::string> dnames)
        : Msg(Msg::ReaddirResp),
          id(id),
          error(error),
          dir_names(dnames)  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializeVector<std::string>(dir_names, serializeString, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgReaddirResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->dir_names =
            unserializeVector<std::string>(unserializeString, rs);
        return res;
    }
};
