#pragma once
#include <gmock/gmock.h>
#include "TTEngineSettings.hpp"

class TTEngineSettingsMock : public TTEngineSettings {
public:
    TTEngineSettingsMock() : TTEngineSettings() {}
    MOCK_METHOD(const TTContactsSettings&, getContactsSettings, (), (const, override));
    MOCK_METHOD(const TTChatSettings&, getChatSettings, (), (const, override));
    MOCK_METHOD(const TTTextBoxSettings&, getTextBoxSettings, (), (const, override));
    MOCK_METHOD(const std::string&, getNickname, (), (const, override));
    MOCK_METHOD(const std::string&, getIdentity, (), (const, override));
    MOCK_METHOD(const TTNetworkInterface&, getInterface, (), (const, override));
    MOCK_METHOD(const std::deque<std::string>&, getNeighbors, (), (const, override));
};