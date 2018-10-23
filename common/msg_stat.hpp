#pragma once
#include "msg_base.hpp"

class MsgStat : public Msg
{
public:
    int32_t id;
    std::string filename;

public:
    MsgStat() : Msg(Msg::Stat), id(0), filename() {}
    MsgStat(int32_t id, std::string filename)
        : Msg(Msg::Stat), id(id), filename(std::move(filename))
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
        auto res = std::make_unique<MsgStat>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        return res;
    }
};

class MsgStatResp : public Msg
{
public:
    int32_t id;
    int32_t error;
// make sure no padding is inserted. The layout should be identical in any
// machine.
#pragma pack(push, 1)
    struct Stat
    {
        int64_t size;
        uint64_t mode;
    } stat;
#pragma pack(pop)
public:
    MsgStatResp()
        : Msg(Msg::StatResp), id(0), error(0), stat()  // more fields

    {
    }
    MsgStatResp(int32_t id, int32_t error, Stat stat)
        : Msg(Msg::StatResp), id(id), error(error), stat(stat)  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializePod<Stat>(stat, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgStatResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->stat = unserializePod<Stat>(rs);
        return res;
    }
};
