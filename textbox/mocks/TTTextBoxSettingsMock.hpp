#pragma once
#include <gmock/gmock.h>
#include "TTTextBoxSettings.hpp"

class TTTextBoxSettingsMock : public TTTextBoxSettings {
public:
    TTTextBoxSettingsMock() : TTTextBoxSettings() {}
    MOCK_METHOD(size_t, getTerminalWidth, (), (const, override));
    MOCK_METHOD(size_t, getTerminalHeight, (), (const, override));
    MOCK_METHOD(std::shared_ptr<TTUtilsNamedPipe>, getNamedPipe, (), (const, override));
};
