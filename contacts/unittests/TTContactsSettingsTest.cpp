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

TEST(TTContactsSettingsTest, UnhappyPathNotEnoughArguments) {
    const int argc = 1;
    const char* const argv[1] = { "a" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid number of arguments")));
}

TEST(TTContactsSettingsTest, UnhappyPathTooManyArguments) {
    const int argc = 5;
    const char* const argv[5] = { "a", "b", "c", "d", "e" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid number of arguments")));
}

TEST(TTContactsSettingsTest, UnhappyPathInvalidTerminalWidth) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "blahblah", "45" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid terminal emulator width=blahblah")));
}

TEST(TTContactsSettingsTest, UnhappyPathInvalidTerminalHeight) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "blahblah", "contacts" };
    EXPECT_THAT([&]() {TTContactsSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTContactsSettings: Invalid terminal emulator height=blahblah")));
}
