#pragma once
#include <stdint.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "serial.hpp"

/* All message classes should derive from the *Msg* class, which contains one
 * field: message type. The derived message can (and should) have its own
 * fields. To implement (un)serialization, a derived Msg class must:
 *
 * 1) add an new entry in enum Msg::Type.
 *
 * 2) the constructor must call Msg(Type type) with the newly added entry
 *
 * 3) implement a member function serializeBody that writes its fields into
 * the stream.
 *
 * 4) implement a static function unserialize that reads its body from the
 * stream and construct an instance of its own.
 *
 * 5) add a pair (msg type, the unserialize function) to the hash map
 * *unserializorLookup*. By doing this, the unserializeMsg function is able to
 * look up the unserialize function from the type read from the stream.
 */

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
    virtual void serializeBody(const SWriter& ws) const = 0;

public:
    void serialize(const SWriter& sw) const
    {
        serializePod<uint32_t>((uint32_t)_type, sw);
        serializeBody(sw);
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
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(flag, ws);
        serializeString(filename, ws);
    }

public:
    static std::unique_ptr<MsgOpen> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgOpen>();
        res->id = unserializePod<int32_t>(rs);
        res->flag = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        return res;
    }
};

void serializeMsg(const Msg& msg, const SWriter& sr);
std::unique_ptr<Msg> unserializeMsg(const SReader& rs);
