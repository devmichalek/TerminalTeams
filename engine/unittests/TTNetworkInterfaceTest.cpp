#include "TTNetworkInterface.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(TTNetworkInterfaceTest, UnhappyPathDefaultConstructor) {
    TTNetworkInterface networkInterface;
    EXPECT_EQ(networkInterface.getName(), std::string{});
    EXPECT_EQ(networkInterface.getIpAddress(), std::string{});
    EXPECT_EQ(networkInterface.getPort(), std::string{});
    EXPECT_EQ(networkInterface.getIpAddressAndPort(), std::string{":"});
}

TEST(TTNetworkInterfaceTest, HappyPath) {
    TTNetworkInterface networkInterface("eno1", "192.168.1.5", "44");
    EXPECT_EQ(networkInterface.getName(), std::string{"eno1"});
    EXPECT_EQ(networkInterface.getIpAddress(), std::string{"192.168.1.5"});
    EXPECT_EQ(networkInterface.getPort(), std::string{"44"});
    EXPECT_EQ(networkInterface.getIpAddressAndPort(), std::string{"192.168.1.5:44"});
}
