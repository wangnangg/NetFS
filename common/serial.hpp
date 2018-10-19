#pragma once
#include <stdint.h>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>

/* For serialize:
 * the object *val* is serialized into the
 * stream. The stream is represented by *SWriter*, which is just a function
 * wrapper, that should write every bytes in buf to the underlying stream.
 * This design is chosen because the server and client will have different
 * stream implementation (client will use FdWriter and server will use POCO).
 * errors are propagated by throwing exceptions from *SWriter*, likely due to
 * network problems.
 *
 * For unserialize:
 * an object T is reconstructed from the stream. *SReader* uses a simliar
 * design. Errors are propagated by throwing exceptions. Exceptions are likely
 * due to 1) system_errors, because of network problem. 2)
 * UnserializeFormatError, due to invalid format in the stream.
 *
 * Example usage can be seen in the unit tests.
 */

using SWriter = std::function<void(const char*, size_t)>;
using SReader = std::function<void(char*, size_t)>;

/* The following two templates are able to serialize and unserialize any POD
 * types.
 *
 * enable_if_t part is only a trick to check if T is POD, nothing interesting
 * here.
 */
template <typename T, std::enable_if_t<std::is_pod<T>::value, int> = 0>
void serializePod(T val, const SWriter& sw)
{
    sw((const char*)&val, sizeof(val));
}

template <typename T, std::enable_if_t<std::is_pod<T>::value, int> = 0>
T unserializePod(const SReader& sr)
{
    T val;
    sr((char*)&val, sizeof(val));
    return val;
}

/* For non-POD type, it must be done case by case.
 */

void serializeString(const std::string& str, const SWriter& sw);
std::string unserializeString(const SReader& sr);

template <typename T>
void serializeVector(
    const std::vector<T>& vec,
    const std::function<void(const T&, const SWriter&)>& serialElement,
    const SWriter& sw)
{
    serializePod<uint64_t>((uint64_t)vec.size(), sw);
    for (const auto& ele : vec)
    {
        serialElement(ele, sw);
    }
}

template <typename T>
std::vector<T> unserializeVector(
    const std::function<T(const SReader&)>& unserialElement,
    const SReader& sr)
{
    size_t size = unserializePod<uint64_t>(sr);
    std::vector<T> vec;
    vec.reserve(size);
    for (size_t i = 0; i < size; i++)
    {
        vec.push_back(unserialElement(sr));
    }
    return vec;
}

// fast specialization for std::vector<char>
void serializeVectorChar(const std::vector<char>& vec, const SWriter& sw);
std::vector<char> unserializeVectorChar(const SReader& sr);

class UnserializeFormatError : public std::runtime_error
{
public:
    UnserializeFormatError(const std::string& tname)
        : std::runtime_error(
              "invalid format in unserializing an object of type: " + tname)
    {
    }
};
