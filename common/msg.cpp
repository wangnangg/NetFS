#include "msg.hpp"

std::unordered_map<Msg::Type, std::unique_ptr<MsgOpen> (*)(const SReader& rs)>
    unserializorLookup = {{Msg::Open, MsgOpen::unserialize}};

void serializeString(const std::string& str, const SWriter& sw)
{
    serializePod<uint32_t>((uint32_t)str.size(), sw);
    if (str.size() > 0)
    {
        sw((const char*)&str[0], str.size());
    }
}

std::string unserializeString(const SReader& sr)
{
    uint32_t str_size = unserializePod<uint32_t>(sr);
    std::string res(str_size, '\0');
    if (str_size > 0)
    {
        sr(&res[0], str_size);
    }
    return res;
}

void serializeMsg(const Msg& msg, const SWriter& sr) { msg.serialize(sr); }

std::unique_ptr<Msg> unserializeMsg(const SReader& rs)
{
    Msg::Type type = (Msg::Type)unserializePod<uint32_t>(rs);
    auto creator = unserializorLookup.at(type);
    return creator(rs);
}
