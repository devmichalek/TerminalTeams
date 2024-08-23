#include "TTEngineSettings.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::ThrowsMessage;
using ::testing::HasSubstr;
using ::testing::NotNull;

TEST(TTEngineSettingsTest, HappyPathNoNeighbors) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.15");
    const std::string hostPort("158");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const int argc = 9;
    const char* const argv[9] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str()
    };
    const TTEngineSettings settings(argc, argv);
    EXPECT_EQ(settings.getNickname(), nickname);
    EXPECT_EQ(settings.getIdentity(), identity);
    EXPECT_EQ(settings.getInterface().getName(), interface);
    EXPECT_EQ(settings.getInterface().getIpAddress(), hostIpAddress);
    EXPECT_EQ(settings.getInterface().getPort(), hostPort);
    EXPECT_EQ(settings.getInterface().getIpAddressAndPort(), hostIpAddressAndPort);
    EXPECT_EQ(settings.getNeighbors().size(), 0);
}

TEST(TTEngineSettingsTest, HappyPathFewNeighbors) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.15");
    const std::string hostPort("158");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const std::string neighbor1IpAddress("192.168.1.85");
    const std::string neighbor2IpAddress("192.168.1.212");
    const std::string neighbor3IpAddress("192.168.1.33");
    const int argc = 12;
    const char* const argv[12] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str(),
        neighbor1IpAddress.c_str(),
        neighbor2IpAddress.c_str(),
        neighbor3IpAddress.c_str()
    };
    const TTEngineSettings settings(argc, argv);
    EXPECT_EQ(settings.getNickname(), nickname);
    EXPECT_EQ(settings.getIdentity(), identity);
    EXPECT_EQ(settings.getInterface().getName(), interface);
    EXPECT_EQ(settings.getInterface().getIpAddress(), hostIpAddress);
    EXPECT_EQ(settings.getInterface().getPort(), hostPort);
    EXPECT_EQ(settings.getInterface().getIpAddressAndPort(), hostIpAddressAndPort);
    EXPECT_EQ(settings.getNeighbors().size(), 3);
    EXPECT_EQ(settings.getNeighbors()[0], neighbor1IpAddress);
    EXPECT_EQ(settings.getNeighbors()[1], neighbor2IpAddress);
    EXPECT_EQ(settings.getNeighbors()[2], neighbor3IpAddress);
}

TEST(TTEngineSettingsTest, UnhappyPathNotEnoughArguments) {
    const int argc = 1;
    const char* const argv[1] = { "a" };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Insufficient number of arguments")));
}

TEST(TTEngineSettingsTest, UnhappyPathInvalidHostIpAddressNoNeighbors1) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("blahblahblah");
    const std::string hostPort("158");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const int argc = 9;
    const char* const argv[9] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str()
    };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Invalid IPv4 address=blahblahblah")));
}

TEST(TTEngineSettingsTest, UnhappyPathInvalidHostIpAddressNoNeighbors2) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.1234");
    const std::string hostPort("158");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const int argc = 9;
    const char* const argv[9] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str()
    };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Invalid IPv4 address=192.168.1.1234")));
}

TEST(TTEngineSettingsTest, HappyPathInvalidNeighborIpAddress1) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.15");
    const std::string hostPort("158");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const std::string neighbor1IpAddress("blahblahblah");
    const int argc = 10;
    const char* const argv[10] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str(),
        neighbor1IpAddress.c_str()
    };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Invalid neighbor IPv4 address=blahblahblah")));
}

TEST(TTEngineSettingsTest, HappyPathInvalidNeighborIpAddress2) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.15");
    const std::string hostPort("158");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const std::string neighbor1IpAddress("192.168.1.1234");
    const int argc = 10;
    const char* const argv[10] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str(),
        neighbor1IpAddress.c_str()
    };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Invalid neighbor IPv4 address=192.168.1.1234")));
}

TEST(TTEngineSettingsTest, UnhappyPathInvalidHostPortNoNeighbors1) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.18");
    const std::string hostPort("blahblah");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const int argc = 9;
    const char* const argv[9] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str()
    };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Invalid port=")));
}

TEST(TTEngineSettingsTest, UnhappyPathInvalidHostPortNoNeighbors2) {
    const std::string nickname("nickname");
    const std::string identity("identity");
    const std::string interface("interface");
    const std::string hostIpAddress("192.168.1.18");
    const std::string hostPort("66666");
    const std::string hostIpAddressAndPort = hostIpAddress + ":" + hostPort;
    const int argc = 9;
    const char* const argv[9] = {
        "/tmp",
        "contacts",
        "chat",
        "textbox",
        nickname.c_str(),
        identity.c_str(),
        interface.c_str(),
        hostIpAddress.c_str(),
        hostPort.c_str()
    };
    EXPECT_THAT([&]() {TTEngineSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngineSettings: Invalid (out of range) port=")));
}
