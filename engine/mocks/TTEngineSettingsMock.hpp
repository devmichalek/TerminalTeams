#pragma once
#include <gmock/gmock.h>
#include "TTEngineSettings.hpp"

class TTEngineSettingsMock : public TTEngineSettings {
public:
    MOCK_METHOD(const std::string&, getNickname, (), (const, override));
    MOCK_METHOD(const std::string&, getIdentity, (), (const, override));
    MOCK_METHOD(const TTNetworkInterface&, getNetworkInterface, (), (const, override));
    MOCK_METHOD(const std::deque<std::string>&, getNeighbors, (), (const, override));
    MOCK_METHOD(const TTAbstractFactory&, getAbstractFactory, (), (const, override));
};