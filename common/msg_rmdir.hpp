#pragma once
#include "msg_base.hpp"

class MsgRmdir : public Msg
{
public:
    int32_t id;
    std::string filename;
    // more fields here
public:
    MsgRmdir() : Msg(Msg::Rmdir), id(0), filename()  // more fields

    {
    }
    MsgRmdir(int32_t id, std::string filename)
        : Msg(Msg::Rmdir), id(id), filename(filename)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(filename, ws);
        // your code here
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgRmdir>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        // your code here
        return res;
    }
};

class MsgRmdirResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    // more fields here
public:
    MsgRmdirResp()
        : Msg(Msg::RmdirResp), id(0), error(0)  // more
                                                // fields

    {
    }
    MsgRmdirResp(int32_t id, int32_t err)
        : Msg(Msg::RmdirResp), id(id), error(err)  // more fields
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
        auto res = std::make_unique<MsgRmdirResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        // your code here
        return res;
    }
};
