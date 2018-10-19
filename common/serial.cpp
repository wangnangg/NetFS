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

void serializeVectorChar(const std::vector<char>& vec, const SWriter& sw)
{
    serializePod<uint64_t>((uint64_t)vec.size(), sw);
    if (vec.size() > 0)
    {
        sw(&vec[0], vec.size());
    }
}
std::vector<char> unserializeVectorChar(const SReader& sr)
{
    uint64_t size = unserializePod<uint64_t>(sr);
    auto vec = std::vector<char>(size);
    if (size > 0)
    {
        sr(&vec[0], size);
    }
    return vec;
}
