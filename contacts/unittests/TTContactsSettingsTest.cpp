#include "TTContactsSettings.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::ThrowsMessage;
using ::testing::HasSubstr;
using ::testing::NotNull;

TEST(TTContactsSettingsTest, HappyPath) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "45", "contacts" };
    const TTContactsSettings settings(argc, argv);
    EXPECT_EQ(settings.getTerminalWidth(), 90);
    EXPECT_EQ(settings.getTerminalHeight(), 45);
    EXPECT_TRUE(settings.getSharedMemory() != nullptr);
}

TEST(TTContactsSettingsTest, NotEnoughArguments) {
    const int argc = 1;
    const char* const argv[1] = { "a" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: invalid number of arguments")));
}

TEST(TTContactsSettingsTest, TooManyArguments) {
    const int argc = 5;
    const char* const argv[5] = { "a", "b", "c", "d", "e" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: invalid number of arguments")));
}

TEST(TTContactsSettingsTest, InvalidTerminalWidth) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "blahblah", "45", "contacts" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: invalid terminal emulator width=blahblah")));
}

TEST(TTContactsSettingsTest, InvalidTerminalHeight) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "blahblah", "contacts" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: invalid terminal emulator height=blahblah")));
}
