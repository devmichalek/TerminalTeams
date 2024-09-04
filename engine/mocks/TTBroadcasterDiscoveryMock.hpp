#pragma once
#include <gmock/gmock.h>
#include "TTBroadcasterDiscovery.hpp"
#include "TTContactsHandlerMock.hpp"
#include "TTChatHandlerMock.hpp"
#include "TTNeighborsStubMock.hpp"

class TTBroadcasterDiscoveryMock : public TTBroadcasterDiscovery {
public:
    TTBroadcasterDiscoveryMock() : TTBroadcasterDiscovery(mContactsHandler, mChatHandler, mNeighborsStub, {}, {}) {}
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
    MOCK_METHOD(bool, handleGreet, (const TTGreetRequest& request), (override));
    MOCK_METHOD(bool, handleHeartbeat, (const TTHeartbeatRequest& request), (override));
    MOCK_METHOD(std::string, getNickname, (), (const, override));
    MOCK_METHOD(std::string, getIdentity, (), (const, override));
    MOCK_METHOD(std::string, getIpAddressAndPort, (), (const, override));
private:
    TTContactsHandlerMock mContactsHandler;
    TTChatHandlerMock mChatHandler;
    TTNeighborsStubMock mNeighborsStub;
};
