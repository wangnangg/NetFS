#include "serial.hpp"

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
