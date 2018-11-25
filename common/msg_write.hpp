#pragma once

#include "msg_base.hpp"

class MsgWrite : public Msg
{
public:
    int32_t id;
    std::string filename;
    int64_t offset;
    std::vector<char> data;
    // more fields here
public:
    MsgWrite()
        : Msg(Msg::Write),
          id(0),
          filename(),
          offset(0),
          data()  // more fields

    {
    }
    MsgWrite(int32_t id, std::string filename, int64_t offset,
             std::vector<char> data)
        : Msg(Msg::Write),
          id(id),
          filename(std::move(filename)),
          offset(offset),
          data(std::move(data))
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializeString(filename, ws);
        serializePod<int64_t>(offset, ws);
        serializeVectorChar(data, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgWrite>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        res->offset = unserializePod<int64_t>(rs);
        res->data = unserializeVectorChar(rs);
        return res;
    }
};

class MsgWriteResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    FileTime before_change;
    FileTime after_change;
    // more fields here
public:
    MsgWriteResp() : Msg(Msg::WriteResp), id(0), error(0)  // more fields

    {
    }
    MsgWriteResp(int32_t id, int32_t error, FileTime before, FileTime after)
        : Msg(Msg::WriteResp),
          id(id),
          error(error),
          before_change(before),
          after_change(after)
    // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializePod<FileTime>(before_change, ws);
        serializePod<FileTime>(after_change, ws);
        // your code here
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgWriteResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->before_change = unserializePod<FileTime>(rs);
        res->after_change = unserializePod<FileTime>(rs);
        // your code here
        return res;
    }
};
