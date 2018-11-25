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
#include "time.hpp"

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
        Access = 0,
        AccessResp,
        Statfs,
        StatfsResp,
        Create,
        CreateResp,
        Stat,
        StatResp,
        Readdir,
        ReaddirResp,
        Read,
        ReadResp,
        Write,
        WriteResp,
        Truncate,
        TruncateResp,
        Unlink,
        UnlinkResp,
        Rmdir,
        RmdirResp,
        Mkdir,
        MkdirResp,
        Rename,
        RenameResp
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
