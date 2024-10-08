#pragma once
#include <gmock/gmock.h>
#include "TTChatHandler.hpp"

class TTChatHandlerMock : public TTChatHandler {
public:
    TTChatHandlerMock() : TTChatHandler() {}
    MOCK_METHOD(bool, send, (size_t, const std::string&, TTChatTimestamp), (override));
    MOCK_METHOD(bool, receive, (size_t, const std::string&, TTChatTimestamp), (override));
    MOCK_METHOD(bool, select, (size_t), (override));
    MOCK_METHOD(bool, create, (size_t), (override));
    MOCK_METHOD(bool, size, (), (const, override));
    MOCK_METHOD(std::optional<TTChatEntries>, get, (size_t), (const, override));
    MOCK_METHOD(std::optional<size_t>, current, (), (const, override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, isStopped, (), (const, override));
};
