#include "net.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

TEST(net, parse_ip)
{
    uint32_t ip = parseIpAddr("1.2.3.4");
    ASSERT_EQ(ip, (1 << 24) + (2 << 16) + (3 << 8) + 4);
}

TEST(net, parse_ip_invalid)
{
    ASSERT_THROW(parseIpAddr("1.2.3.434"), std::invalid_argument);
    ASSERT_THROW(parseIpAddr("1.2.3"), std::invalid_argument);
    ASSERT_THROW(parseIpAddr("1.2.3.4.5"), std::invalid_argument);
    ASSERT_THROW(parseIpAddr(""), std::invalid_argument);
}
