#include "TTContactsSettings.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::ThrowsMessage;
using ::testing::HasSubstr;
using ::testing::NotNull;

TEST(TTContactsSettingsTest, HappyPath) {
    const int argc = 8;
    const char* const argv[8] = { "/tmp", "90", "45", "contacts", "nickname", "identity", "192.168.1.0", "183" };
    const TTContactsSettings settings(argc, argv);
    EXPECT_EQ(settings.getTerminalWidth(), 90);
    EXPECT_EQ(settings.getTerminalHeight(), 45);
    EXPECT_TRUE(settings.getSharedMemory() != nullptr);
    EXPECT_EQ(settings.getNickname(), "nickname");
    EXPECT_EQ(settings.getIdentity(), "identity");
    EXPECT_EQ(settings.getIpAddress(), "192.168.1.0");
    EXPECT_EQ(settings.getPort(), "183");
}

TEST(TTContactsSettingsTest, NotEnoughArguments) {
    const int argc = 1;
    const char* const argv[1] = { "a" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid number of arguments")));
}

TEST(TTContactsSettingsTest, TooManyArguments) {
    const int argc = 9;
    const char* const argv[9] = { "a", "b", "c", "d", "e", "f", "g", "h", "i" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid number of arguments")));
}

TEST(TTContactsSettingsTest, InvalidTerminalWidth) {
    const int argc = 8;
    const char* const argv[8] = { "/tmp", "blahblah", "45", "contacts", "nickname", "identity", "192.168.1.0", "15" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid terminal emulator width=blahblah")));
}

TEST(TTContactsSettingsTest, InvalidTerminalHeight) {
    const int argc = 8;
    const char* const argv[8] = { "/tmp", "90", "blahblah", "contacts", "nickname", "identity", "192.168.1.0", "15" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid terminal emulator height=blahblah")));
}
