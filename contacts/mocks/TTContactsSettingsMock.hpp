#pragma once
#include <gmock/gmock.h>
#include "TTContactsSettings.hpp"

class TTContactsSettingsMock : public TTContactsSettings {
public:
    TTContactsSettingsMock() : TTContactsSettings() {}
    MOCK_METHOD(size_t, getTerminalWidth, (), (const, override));
    MOCK_METHOD(size_t, getTerminalHeight, (), (const, override));
    MOCK_METHOD(std::shared_ptr<TTUtilsSharedMem>, getSharedMemory, (), (const, override));
};
