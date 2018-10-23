#pragma once
#include "msg_base.hpp"

class MsgRead : public Msg
{
public:
    int32_t id;
    std::string filename;
    int64_t offset;
    int64_t size;
    // more fields here
public:
    MsgRead() : Msg(Msg::Read), id(0), offset(0), size(0)  // more fields

    {
    }
    MsgRead(int32_t id, std::string filename, int64_t offset, int64_t size)
        : Msg(Msg::Read),
          id(id),
          filename(std::move(filename)),
          offset(offset),
          size(size)  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(filename, ws);
        serializePod<int64_t>(offset, ws);
        serializePod<int64_t>(size, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgRead>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        res->offset = unserializePod<int64_t>(rs);
        res->size = unserializePod<int64_t>(rs);
        return res;
    }
};

class MsgReadResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    std::vector<char> data;
    // more fields here
public:
    MsgReadResp()
        : Msg(Msg::ReadResp), id(0), error(0), data()  // more fields

    {
    }
    MsgReadResp(int32_t id, int32_t error, std::vector<char> data)
        : Msg(Msg::ReadResp),
          id(id),
          error(error),
          data(std::move(data))  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializeVectorChar(data, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgReadResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->data = unserializeVectorChar(rs);
        return res;
    }
};
