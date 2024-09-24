#pragma once
#include "TTNeighborsMessage.hpp"
#include "TerminalTeams.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using TTNeighborsChatStubIf = tt::NeighborsChat::StubInterface;
using TTNeighborsDiscoveryStubIf = tt::NeighborsDiscovery::StubInterface;
using TTUniqueChatStub = std::unique_ptr<TTNeighborsChatStubIf>;
using TTUniqueDiscoveryStub = std::unique_ptr<TTNeighborsDiscoveryStubIf>;

class TTNeighborsStub {
public:
    TTNeighborsStub() = default;
    virtual ~TTNeighborsStub() = default;
    TTNeighborsStub(const TTNeighborsStub&) = delete;
    TTNeighborsStub(TTNeighborsStub&&) = delete;
    TTNeighborsStub& operator=(const TTNeighborsStub&) = delete;
    TTNeighborsStub& operator=(TTNeighborsStub&&) = delete;
    [[nodiscard]] virtual TTUniqueChatStub createChatStub(const std::string& ipAddressAndPort) const;
    [[nodiscard]] virtual TTUniqueDiscoveryStub createDiscoveryStub(const std::string& ipAddressAndPort) const;
    [[nodiscard]] virtual TTTellResponse sendTell(TTNeighborsChatStubIf& stub, const TTTellRequest& rhs) const;
    [[nodiscard]] virtual TTNarrateResponse sendNarrate(TTNeighborsChatStubIf& stub, const TTNarrateRequest& rhs) const;
    [[nodiscard]] virtual TTGreetResponse sendGreet(TTNeighborsDiscoveryStubIf& stub, const TTGreetRequest& rhs) const;
    [[nodiscard]] virtual TTHeartbeatResponse sendHeartbeat(TTNeighborsDiscoveryStubIf& stub, const TTHeartbeatRequest& rhs) const;
};
