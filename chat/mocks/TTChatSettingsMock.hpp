#pragma once
#include <gmock/gmock.h>
#include "TTChatSettings.hpp"

class TTChatSettingsMock : public TTChatSettings {
public:
    TTChatSettingsMock() : TTChatSettings() {}
    MOCK_METHOD(size_t, getTerminalWidth, (), (const, override));
    MOCK_METHOD(size_t, getTerminalHeight, (), (const, override));
    MOCK_METHOD(std::shared_ptr<TTUtilsMessageQueue>, getPrimaryMessageQueue, (), (const, override));
    MOCK_METHOD(std::shared_ptr<TTUtilsMessageQueue>, getSecondaryMessageQueue, (), (const, override));
    MOCK_METHOD(double, getRatio, (), (const, override));
};
