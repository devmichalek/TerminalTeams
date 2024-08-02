#pragma once
#include <gmock/gmock.h>
#include "TTChat.hpp"

class TTChatMock : public TTChat {
public:
    TTChatMock() : TTChat() {}
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
};
