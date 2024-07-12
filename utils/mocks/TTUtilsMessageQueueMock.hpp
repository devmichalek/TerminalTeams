#pragma once

#include "TTUtilsMessageQueue.hpp"

class TTUtilsMessageQueueMock : public TTUtilsMessageQueue {
public:
    MOCK_METHOD(bool, create, (), (override));
    MOCK_METHOD(bool, alive, (), (const, override));
    MOCK_METHOD(bool, open, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, receive, (char* message, long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, send, (const char* message, long attempts, long timeoutMs), (override));
};