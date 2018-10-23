#pragma once
#include "msg_base.hpp"

class MsgUnlink : public Msg
{
public:
    int32_t id;
    std::string filename;
    // more fields here
public:
    MsgUnlink() : Msg(Msg::Unlink), id(0), filename()  // more fields

    {
    }
    MsgUnlink(int32_t id, std::string filename)
        : Msg(Msg::Unlink), id(id), filename(filename)
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
        auto res = std::make_unique<MsgUnlink>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        // your code here
        return res;
    }
};

class MsgUnlinkResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    // more fields here
public:
    MsgUnlinkResp()
        : Msg(Msg::UnlinkResp), id(0), error(0)  // more
                                                 // fields

    {
    }
    MsgUnlinkResp(int32_t id, int32_t err)
        : Msg(Msg::UnlinkResp), id(id), error(err)  // more fields
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
        auto res = std::make_unique<MsgUnlinkResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        // your code here
        return res;
    }
};
