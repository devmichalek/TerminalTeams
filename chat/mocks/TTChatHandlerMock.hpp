#pragma once
#include <gmock/gmock.h>
#include "TTChatHandler.hpp"

class TTChatHandlerMock : public TTChatHandler {
public:
    TTChatHandlerMock() : TTChatHandler() {}
    MOCK_METHOD(bool, send, (size_t, const std::string&, TTChatTimestamp), (override));
    MOCK_METHOD(bool, receive, (size_t, const std::string&, TTChatTimestamp), (override));
    MOCK_METHOD(bool, clear, (size_t), (override));
    MOCK_METHOD(bool, create, (size_t), (override));
    MOCK_METHOD(bool, size, (), (const, override));
    MOCK_METHOD(std::optional<const TTChatEntries&>, get, (size_t), (const, override));
    MOCK_METHOD(std::optional<size_t>, current, (size_t), (const, override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
};
