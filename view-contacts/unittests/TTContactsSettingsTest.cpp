#include "TTContactsSettings.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::ThrowsMessage;
using ::testing::HasSubstr;

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

