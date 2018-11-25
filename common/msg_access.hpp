#pragma once
#include "msg_base.hpp"
class MsgAccess : public Msg
{
public:
    int32_t id;
    std::string filename;

public:
    MsgAccess() : Msg(Msg::Access), id(0), filename() {}
    MsgAccess(int32_t id, std::string filename)
        : Msg(Msg::Access), id(id), filename(std::move(filename))
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
        auto res = std::make_unique<MsgAccess>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        return res;
    }
};

class MsgAccessResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    FileTime time;

public:
    MsgAccessResp() : Msg(Msg::AccessResp), id(0) {}
    MsgAccessResp(int32_t id, int32_t error, const FileTime& time)
        : Msg(Msg::AccessResp), id(id), error(error), time(time)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializePod<FileTime>(time, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgAccessResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->time = unserializePod<FileTime>(rs);
        return res;
    }
};
