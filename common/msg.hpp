#pragma once
#include <stdint.h>
#include <sys/stat.h>
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

/* template

   class Msg#NAME : public Msg
   {
   public:
   int32_t id;
   //more fields here
   public:
   Msg#NAME() : Msg(Msg::#NAME), id(0) //more fields

   {}
   Msg#NAME(int32_t id)
   : Msg(Msg::#NAME), id(id) //more fields
   {
   }

   protected:
   virtual void serializeBody(const SWriter& ws) const
   {
   serializePod<int32_t>(id, ws);
   //your code here
   }

   public:
   static std::unique_ptr<Msg> unserialize(const SReader& rs)
   {
   auto res = std::make_unique<Msg#NAME>();
   res->id = unserializePod<int32_t>(rs);
   //your code here
   return res;
   }
   };
*/

class Msg
{
public:
    enum Type
    {
        Open = 0,
        OpenResp,
        Stat,
        StatResp,
        Readdir,
        ReaddirResp,
        Read,
        ReadResp,
        Write,
        WriteResp,
        Truncate,
        TruncateResp
    } type;

protected:
    Msg(Type type) : type(type) {}
    virtual void serializeBody(const SWriter& ws) const = 0;

public:
    void serialize(const SWriter& sw) const
    {
        serializePod<uint32_t>((uint32_t)type, sw);
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

public:
    MsgOpenResp() : Msg(Msg::OpenResp), id(0) {}
    MsgOpenResp(int32_t id, int32_t error)
        : Msg(Msg::OpenResp), id(id), error(error)
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
        auto res = std::make_unique<MsgOpenResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        return res;
    }
};

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

class MsgReaddir : public Msg
{
public:
    int32_t id;
    std::string filename;
    // more fields here
public:
    MsgReaddir() : Msg(Msg::Readdir), id(0), filename()  // more fields

    {
    }
    MsgReaddir(int32_t id, std::string filename)
        : Msg(Msg::Readdir),
          id(id),
          filename(std::move(filename))  // more fields
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
        auto res = std::make_unique<MsgReaddir>();
        res->id = unserializePod<int32_t>(rs);
        res->filename = unserializeString(rs);
        return res;
    }
};

class MsgReaddirResp : public Msg
{
public:
    int32_t id;
    int32_t error;
    std::vector<std::string> dir_names;
    // more fields here
public:
    MsgReaddirResp()
        : Msg(Msg::ReaddirResp), id(0), error(), dir_names()  // more fields

    {
    }
    MsgReaddirResp(int32_t id, int32_t error, std::vector<std::string> dnames)
        : Msg(Msg::ReaddirResp),
          id(id),
          error(error),
          dir_names(dnames)  // more fields
    {
    }

protected:
    virtual void serializeBody(const SWriter& ws) const
    {
        serializePod<int32_t>(id, ws);
        serializePod<int32_t>(error, ws);
        serializeVector<std::string>(dir_names, serializeString, ws);
    }

public:
    static std::unique_ptr<Msg> unserialize(const SReader& rs)
    {
        auto res = std::make_unique<MsgReaddirResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        res->dir_names =
            unserializeVector<std::string>(unserializeString, rs);
        return res;
    }
};

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
    // more fields here
public:
    MsgWriteResp() : Msg(Msg::WriteResp), id(0), error(0)  // more fields

    {
    }
    MsgWriteResp(int32_t id, int32_t error)
        : Msg(Msg::WriteResp), id(id), error(error)  // more fields
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
        auto res = std::make_unique<MsgWriteResp>();
        res->id = unserializePod<int32_t>(rs);
        res->error = unserializePod<int32_t>(rs);
        // your code here
        return res;
    }
};

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

void serializeMsg(const Msg& msg, const SWriter& sr);
std::unique_ptr<Msg> unserializeMsg(const SReader& rs);
