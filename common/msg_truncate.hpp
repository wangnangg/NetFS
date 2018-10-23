#pragma once
#include "msg_base.hpp"

class MsgTruncate : public Msg
{
public:
    int32_t id;
    std::string filename;
    int64_t offset;
    // more fields here
public:
    MsgTruncate()
        : Msg(Msg::Truncate), id(0), filename(), offset(0)  // more fields

    {
    }
    MsgTruncate(int32_t id, std::string filename, int64_t off)
        : Msg(Msg::Truncate),
          id(id),
          filename(filename),
          offset(off)  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(filename, ws);
        serializePod<int64_t>(offset, ws);
        // your code here
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgTruncate>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        res->offset = unserializePod<int64_t>(rs);
        // your code here
        return res;
    }
};

class MsgTruncateResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    // more fields here
public:
    MsgTruncateResp()
        : Msg(Msg::TruncateResp), id(0), error(0)  // more
                                                   // fields

    {
    }
    MsgTruncateResp(int32_t id, int32_t err)
        : Msg(Msg::TruncateResp), id(id), error(err)  // more fields
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
        auto res = std::make_unique<MsgTruncateResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        // your code here
        return res;
    }
};
