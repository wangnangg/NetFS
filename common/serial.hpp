#pragma once
#include <stdint.h>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>

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

using SWriter = std::function<void(const char*, size_t)>;
using SReader = std::function<void(char*, size_t)>;

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

void serializeString(const std::string& str, const SWriter& sw);
std::string unserializeString(const SReader& sr);

class UnserializeFormatError : public std::runtime_error
{
public:
    UnserializeFormatError(const std::string& tname)
        : std::runtime_error(
              "invalid format in unserializing an object of type: " + tname)
    {
    }
};
