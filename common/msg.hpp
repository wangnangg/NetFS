#pragma once
#include <stdint.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "stream.hpp"

/* For serialize:
 * the object *val* is serialized into the
 * stream. errors are propagated throw exceptions. Exceptions are
 * system_errors, which are due to network problems.
 *
 * For unserialize:
 * an object T is reconstructed from the stream. errors are propagated throw
 * exceptions. Exceptions are 1) system_errors, because of network problem. 2)
 * UnserializeFormatError, due to invalid format in the stream.
 */

void serialize_uint32(uint32_t val, FdWriter& ws);
uint32_t unserialize_uint32(FdReader& rs);

void serialize_int32(int32_t val, FdWriter& ws);
int32_t unserialize_int32(FdReader& rs);

void serialize_string(const std::string& str, FdWriter& ws);
std::string unserialize_string(FdReader& rs);

class Msg
{
public:
    enum Type
    {
        Open = 0,
        OpenResp,
    };

private:
    Type _type;

protected:
    Msg(Type type) : _type(type) {}
    virtual void serializeBody(FdWriter& ws) const = 0;

public:
    void serialize(FdWriter& ws) const
    {
        serialize_uint32((uint32_t)_type, ws);
        serializeBody(ws);
    }
    virtual ~Msg() = default;
};

class MsgOpen : public Msg
{
public:
    int32_t id;
    int32_t flag;
    std::string filename;

public:
    MsgOpen() : Msg(Msg::Open), id(0), flag(0), filename() {}
    MsgOpen(int32_t id, int32_t flag, std::string filename)
        : Msg(Msg::Open), id(id), flag(flag), filename(filename)
    {
    }

protected:
    virtual void serializeBody(FdWriter& ws) const
    {
        serialize_int32(id, ws);
        serialize_int32(flag, ws);
        serialize_string(filename, ws);
    }

public:
    static std::unique_ptr<MsgOpen> unserialize(FdReader& rs)
    {
        auto res = std::make_unique<MsgOpen>();
        res->id = unserialize_int32(rs);
        res->flag = unserialize_int32(rs);
        res->filename = unserialize_string(rs);
        return res;
    }
};

class UnserializeFormatError : public std::runtime_error
{
public:
    UnserializeFormatError(const std::string& tname)
        : std::runtime_error(
              "invalid format in unserializing an object of type: " + tname)
    {
    }
};

void serialize_msg(const Msg& msg, FdWriter& ws);
std::unique_ptr<Msg> unserialize_msg(FdReader& rs);
