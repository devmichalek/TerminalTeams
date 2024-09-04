#pragma once
#include <gmock/gmock.h>
#include "TTBroadcasterChat.hpp"
#include "TTContactsHandlerMock.hpp"
#include "TTChatHandlerMock.hpp"
#include "TTNeighborsStubMock.hpp"

class TTBroadcasterChatMock : public TTBroadcasterChat {
public:
    TTBroadcasterChatMock() : TTBroadcasterChat(mContactsHandler, mChatHandler, mNeighborsStub, {}) {}
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
    MOCK_METHOD(bool, handleSend, (const std::string& message), (override));
    MOCK_METHOD(bool, handleReceive, (const TTTellRequest& request), (override));
    MOCK_METHOD(bool, handleReceive, (const TTNarrateRequest& request), (override));
    MOCK_METHOD(std::string, getNickname, (), (const, override));
private:
    TTContactsHandlerMock mContactsHandler;
    TTChatHandlerMock mChatHandler;
    TTNeighborsStubMock mNeighborsStub;
};
