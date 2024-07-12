#pragma once

#include <gmock/gmock.h>
#include "TTUtilsNamedPipe.hpp"

class TTTUtilsNamedPipeMock : public TTUtilsNamedPipe {
public:
    MOCK_METHOD(bool, alive, (), (const, override));
    MOCK_METHOD(bool, create, (), (override));
    MOCK_METHOD(bool, open, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, receive, (char* message), (override));
    MOCK_METHOD(bool, send, (const char* message), (override));
};
