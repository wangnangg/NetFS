#include "net.hpp"
#include <iostream>
#include <stdexcept>

uint32_t parseIpAddr(const std::string& ip_str)
{
    const char* start = ip_str.c_str();
    char* end;
    uint32_t ip = 0;
    int offset = 32;
    for (long i = std::strtol(start, &end, 10); start != end;
         i = std::strtol(start, &end, 10))
    {
        if (i < 0 || i > 255)
        {
            throw std::invalid_argument(
                "invalid ip address: not in [0, 255].");
        }
        offset -= 8;
        if (offset < 0)
        {
            throw std::invalid_argument(
                "invalid ip address: more than four integers.");
        }
        ip = ip | (i << offset);

        while (end != &*ip_str.end() && !isdigit(*end))
        {
            end += 1;
        }
        start = end;
    }
    if (offset != 0)
    {
        throw std::invalid_argument(
            "invalid ip address: less than four integers.");
    }
    if (end != &*ip_str.end())
    {
        throw std::invalid_argument(
            "invalid ip address: extra unrecognized characters.");
    }
    return ip;
}

uint16_t parsePort(const std::string& port_str)
{
    const char* start = port_str.c_str();
    char* end;
    long port = std::strtol(start, &end, 10);
    if (start == end)
    {
        throw std::invalid_argument(
            "port number string is not a valid number.");
    }
    if (port < 0 || port > 65535)
    {
        throw std::invalid_argument("port number out of range [0, 65535]");
    }
    return (uint16_t)port;
}
