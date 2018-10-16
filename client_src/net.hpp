#pragma once
#include <sys/types.h>
#include <string>
#include <vector>

/* Parse IPv4 address in the form of 192.168.1.2
 * the return value (uint32_T) is the IP address in host byte order (for x86,
 * little endian).
 * If ip_str is not a valid IP address, std::invalid_argument
 * will be thrown.
 */
uint32_t parseIpAddr(const std::string& ip_str);

/* Parse port number. Must be in the range of [0, 65535].
 * std::invalid_argument will be thrown upon error.
 */
uint16_t parsePort(const std::string& port_str);

class NetIO
{
public:
    /* ip and port is in host byte order.
     */
    NetIO(uint32_t ip, uint16_t port);
    void read(std::vector<char>& buf, size_t offset, size_t size);

    /* if size < 0, all data in buf will be written
     */
    void write(const std::vector<char>& buf, size_t offset = 0,
               ssize_t size = -1);
};
