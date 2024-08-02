#pragma once
#include <gmock/gmock.h>
#include "TTChatHandler.hpp"

class TTChatHandlerMock : public TTChatHandler {
public:
    TTChatHandlerMock() : TTChatHandler() {}
    MOCK_METHOD(bool, send, (size_t, std::string, TTChatTimestamp), (override));
    MOCK_METHOD(bool, receive, (size_t, std::string, TTChatTimestamp), (override));
    MOCK_METHOD(bool, clear, (size_t), (override));
    MOCK_METHOD(bool, create, (size_t), (override));
    MOCK_METHOD(const TTChatEntries&, create, (size_t), (const, override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
};
