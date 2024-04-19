#pragma once
#include <gmock/gmock.h>
#include "TTContactsSettings.hpp"

class TTContactsSettingsMock : public TTContactsSettings {
 public:
    MOCK_METHOD(size_t, getTerminalWidth, (), (const, override));
    MOCK_METHOD(size_t, getTerminalHeight, (), (const, override));
    MOCK_METHOD(std::unique_ptr<TTContactsConsumer>, getConsumer, (), (const, override));
};
