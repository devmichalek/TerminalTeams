#include "TTChatSettings.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::ThrowsMessage;
using ::testing::HasSubstr;
using ::testing::NotNull;

TEST(TTChatSettingsTest, HappyPath) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "45", "chat" };
    const TTChatSettings settings(argc, argv);
    EXPECT_EQ(settings.getTerminalWidth(), 90);
    EXPECT_EQ(settings.getTerminalHeight(), 45);
    EXPECT_NE(settings.getPrimaryMessageQueue(), nullptr);
    EXPECT_NE(settings.getSecondaryMessageQueue(), nullptr);
}

TEST(TTChatSettingsTest, NotEnoughArguments) {
    const int argc = 1;
    const char* const argv[1] = { "a" };
    EXPECT_THAT([&]() {TTChatSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTChatSettings: Invalid number of arguments")));
}

TEST(TTChatSettingsTest, TooManyArguments) {
    const int argc = 5;
    const char* const argv[5] = { "a", "b", "c", "d", "e" };
    EXPECT_THAT([&]() {TTChatSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTChatSettings: Invalid number of arguments")));
}

TEST(TTChatSettingsTest, InvalidTerminalWidth) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "blahblah", "45" };
    EXPECT_THAT([&]() {TTChatSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTChatSettings: Invalid terminal emulator width=blahblah")));
}

TEST(TTChatSettingsTest, InvalidTerminalHeight) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "blahblah", "chat" };
    EXPECT_THAT([&]() {TTChatSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTChatSettings: Invalid terminal emulator height=blahblah")));
}
