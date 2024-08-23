#include "TTTextBoxSettings.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::ThrowsMessage;
using ::testing::HasSubstr;
using ::testing::NotNull;

TEST(TTTextBoxSettingsTest, HappyPath) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "45", "/tmp/named-pipe" };
    const TTTextBoxSettings settings(argc, argv);
    EXPECT_EQ(settings.getTerminalWidth(), 90);
    EXPECT_EQ(settings.getTerminalHeight(), 45);
    EXPECT_TRUE(settings.getNamedPipe() != nullptr);
}

TEST(TTTextBoxSettingsTest, UnhappyPathNotEnoughArguments) {
    const int argc = 1;
    const char* const argv[1] = { "a" };
    EXPECT_THAT([&]() {TTTextBoxSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTTextBoxSettings: Invalid number of arguments")));
}

TEST(TTTextBoxSettingsTest, UnhappyPathTooManyArguments) {
    const int argc = 5;
    const char* const argv[5] = { "a", "b", "c", "d", "e" };
    EXPECT_THAT([&]() {TTTextBoxSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTTextBoxSettings: Invalid number of arguments")));
}

TEST(TTTextBoxSettingsTest, UnhappyPathInvalidTerminalWidth) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "blahblah", "45" };
    EXPECT_THAT([&]() {TTTextBoxSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTTextBoxSettings: Invalid terminal emulator width=blahblah")));
}

TEST(TTTextBoxSettingsTest, UnhappyPathInvalidTerminalHeight) {
    const int argc = 4;
    const char* const argv[4] = { "/tmp", "90", "blahblah", "/tmp/named-pipe" };
    EXPECT_THAT([&]() {TTTextBoxSettings(argc, argv);},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTTextBoxSettings: Invalid terminal emulator height=blahblah")));
}
