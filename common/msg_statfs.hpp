#pragma once
#include <sys/statvfs.h>
#include "msg_base.hpp"
class MsgStatfs : public Msg
{
public:
    int32_t id;

public:
    MsgStatfs() : Msg(Msg::Statfs), id(0) {}
    MsgStatfs(int32_t id) : Msg(Msg::Statfs), id(id) {}

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgStatfs>();
        res->id = unserializePod<int32_t>(rs);
        return res;
    }
};

#pragma pack(push, 1)
struct FsStat
{
    uint64_t bsize;
    uint64_t frsize;
    uint64_t blocks;
    uint64_t bfree;
    uint64_t bavail;
    uint64_t files;
    uint64_t ffree;
    uint64_t namemax;
};
#pragma pack(pop)

FsStat makeFsStat(const struct statvfs& st);

class MsgStatfsResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    FsStat stat;

    // make sure no padding is inserted. The layout should be identical in any
    // machine.

public:
    MsgStatfsResp() : Msg(Msg::StatfsResp), id(0), stat() {}
    MsgStatfsResp(int32_t id, int32_t error, const FsStat& st)
        : Msg(Msg::StatfsResp), id(id), error(error), stat(st)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializePod<FsStat>(stat, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgStatfsResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->stat = unserializePod<FsStat>(rs);
        return res;
    }
};
