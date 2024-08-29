#pragma once
#include <gmock/gmock.h>
#include "TTNeighborsStub.hpp"

class TTNeighborsStubMock : public TTNeighborsStub {
public:
    MOCK_METHOD(TTUniqueChatStub, createChatStub, (const std::string& ipAddressAndPort), (const, override));
    MOCK_METHOD(TTUniqueDiscoveryStub, createDiscoveryStub, (const std::string& ipAddressAndPort), (const, override));
    MOCK_METHOD(TTTellResponse, sendTell, (TTNeighborsChatStubIf& stub, const TTTellRequest& rhs), (const, override));
    MOCK_METHOD(TTNarrateResponse, sendNarrate, (TTNeighborsChatStubIf& stub, const TTNarrateRequest& rhs), (const, override));
    MOCK_METHOD(TTGreetResponse, sendGreet, (TTNeighborsDiscoveryStubIf& stub, const TTGreetRequest& rhs), (const, override));
    MOCK_METHOD(TTHeartbeatResponse, sendHeartbeat, (TTNeighborsDiscoveryStubIf& stub, const TTHeartbeatRequest& rhs), (const, override));
};
