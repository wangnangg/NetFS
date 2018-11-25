#pragma once
#include "msg_base.hpp"
class MsgRename : public Msg
{
public:
    int32_t id;
    std::string from;
    std::string to;
    uint32_t flags;

public:
    MsgRename() : Msg(Msg::Rename), id(0), from(), to(), flags() {}
    MsgRename(int32_t id, const std::string& from, const std::string& to,
              int32_t flags)
        : Msg(Msg::Rename), id(id), from(from), to(to), flags(flags)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(from, ws);
        serializeString(to, ws);
        serializePod<uint32_t>(flags, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgRename>();
        res->id = unserializePod<int32_t>(rs);
        res->from = unserializeString(rs);
        res->to = unserializeString(rs);
        res->flags = unserializePod<uint32_t>(rs);
        return res;
    }
};

class MsgRenameResp : public Msg
{
public:
    int32_t id;
    int32_t error;

public:
    MsgRenameResp() : Msg(Msg::RenameResp), id(0) {}
    MsgRenameResp(int32_t id, int32_t error)
        : Msg(Msg::RenameResp), id(id), error(error)
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
        auto res = std::make_unique<MsgRenameResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        return res;
    }
};
