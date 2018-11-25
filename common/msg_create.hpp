#pragma once
#include "msg_base.hpp"
class MsgCreate : public Msg
{
public:
    int32_t id;
    std::string filename;

public:
    MsgCreate() : Msg(Msg::Create), id(0), filename() {}
    MsgCreate(int32_t id, std::string filename)
        : Msg(Msg::Create), id(id), filename(std::move(filename))
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
        auto res = std::make_unique<MsgCreate>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        return res;
    }
};

class MsgCreateResp : public Msg
{
public:
    int32_t id;
    int32_t error;

public:
    MsgCreateResp() : Msg(Msg::CreateResp), id(0) {}
    MsgCreateResp(int32_t id, int32_t error)
        : Msg(Msg::CreateResp), id(id), error(error)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgCreateResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        return res;
    }
};
