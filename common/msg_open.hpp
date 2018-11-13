#pragma once
#include "msg_base.hpp"
class MsgOpen : public Msg
{
public:
    int32_t id;
    int32_t flag;
    std::string filename;
    int32_t mode;

public:
    MsgOpen() : Msg(Msg::Open), id(0), flag(0), filename(), mode() {}
    MsgOpen(int32_t id, int32_t flag, std::string filename, int32_t mode)
        : Msg(Msg::Open),
          id(id),
          flag(flag),
          filename(std::move(filename)),
          mode(mode)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(flag, ws);
        serializeString(filename, ws);
        serializePod<int32_t>(mode, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgOpen>();
        res->id = unserializePod<int32_t>(rs);
        res->flag = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        res->mode = unserializePod<int32_t>(rs);
        return res;
    }
};

class MsgOpenResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    int64_t mtime_sec;
    int64_t mtime_nsec;

public:
    MsgOpenResp() : Msg(Msg::OpenResp), id(0) {}
    MsgOpenResp(int32_t id, int32_t error, int64_t mtime_sec,
                int64_t mtime_nsec)
        : Msg(Msg::OpenResp),
          id(id),
          error(error),
          mtime_sec(mtime_sec),
          mtime_nsec(mtime_nsec)
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializePod<int64_t>(mtime_sec, ws);
        serializePod<int64_t>(mtime_nsec, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgOpenResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->mtime_sec = unserializePod<int64_t>(rs);
        res->mtime_nsec = unserializePod<int64_t>(rs);
        return res;
    }
};
