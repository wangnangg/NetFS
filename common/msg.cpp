#include "msg.hpp"

std::unordered_map<Msg::Type, std::unique_ptr<MsgOpen> (*)(FdReader& rs)>
    unserializorLookup = {{Msg::Open, MsgOpen::unserialize}};

void serialize_uint32(uint32_t val, FdWriter& ws)
{
    ws.write((char*)&val, sizeof(val));
}
uint32_t unserialize_uint32(FdReader& rs)
{
    uint32_t val;
    rs.read((char*)&val, sizeof(val));
    return val;
}

void serialize_int32(int32_t val, FdWriter& ws)
{
    ws.write((char*)&val, sizeof(val));
}
int32_t unserialize_int32(FdReader& rs)
{
    int32_t val;
    rs.read((char*)&val, sizeof(val));
    return val;
}

void serialize_string(const std::string& str, FdWriter& ws)
{
    serialize_uint32((uint32_t)str.size(), ws);
    if (str.size() > 0)
    {
        ws.write(&str[0], str.size());
    }
}

const size_t MAX_STRING_LENGTH = 1000;
std::string unserialize_string(FdReader& rs)
{
    uint32_t str_size;
    rs.read((char*)&str_size, sizeof(str_size));
    if (str_size > MAX_STRING_LENGTH)
    {
        throw UnserializeFormatError("string (too long)");
    }
    std::string res(str_size, '\0');
    if (str_size > 0)
    {
        rs.read(&res[0], str_size);
    }
    return res;
}

void serialize_msg(const Msg& msg, FdWriter& ws) { msg.serialize(ws); }

std::unique_ptr<Msg> unserialize_msg(FdReader& rs)
{
    Msg::Type type = (Msg::Type)unserialize_uint32(rs);
    auto creator = unserializorLookup.at(type);
    return creator(rs);
}
