#pragma once
#include "msg_base.hpp"

class MsgMkdir : public Msg
{
public:
    int32_t id;
    std::string filename;
    int32_t mode;

    // more fields here
public:
    MsgMkdir() : Msg(Msg::Mkdir), id(0), filename(), mode(0)  // more fields

    {
    }
    MsgMkdir(int32_t id, std::string filename, int32_t mode)
        : Msg(Msg::Mkdir), id(id), filename(filename), mode(mode)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(filename, ws);
        serializePod<int32_t>(mode, ws);
        // your code here
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgMkdir>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        res->mode = unserializePod<int32_t>(rs);
        // your code here
        return res;
    }
};

class MsgMkdirResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    // more fields here
public:
    MsgMkdirResp()
        : Msg(Msg::MkdirResp), id(0), error(0)  // more
                                                // fields

    {
    }
    MsgMkdirResp(int32_t id, int32_t err)
        : Msg(Msg::MkdirResp), id(id), error(err)  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        // your code here
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgMkdirResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        // your code here
        return res;
    }
};
