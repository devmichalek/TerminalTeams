#pragma once
#include "TerminalTeams.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using TTNeighborsChatStubIf = tt::NeighborsChat::StubInterface;
using TTNeighborsDiscoveryStubIf = tt::NeighborsDiscovery::StubInterface;
using TTUniqueChatStub = std::unique_ptr<TTNeighborsChatStubIf>;
using TTUniqueDiscoveryStub = std::unique_ptr<TTNeighborsDiscoveryStubIf>;

struct TTTellRequest {
    bool operator==(const TTTellRequest& rhs) const {
        return identity == rhs.identity && message == rhs.message;
    }
    std::string identity;
    std::string message;
};

struct TTTellResponse {
    bool status;
};

struct TTNarrateRequest {
    bool operator==(const TTNarrateRequest& rhs) const {
        return identity == rhs.identity && messages == rhs.messages;
    }
    std::string identity;
    std::deque<std::string> messages;
};

struct TTNarrateResponse {
    bool status;
};

struct TTGreetRequest {
    bool operator==(const TTGreetRequest& rhs) const {
        return nickname == rhs.nickname && identity == rhs.identity && ipAddressAndPort == rhs.ipAddressAndPort;
    }
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTGreetResponse {
    TTGreetResponse() = default;
    bool status;
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTHeartbeatRequest {
    bool operator==(const TTHeartbeatRequest& rhs) const {
        return identity == rhs.identity;
    }
    std::string identity;
};

struct TTHeartbeatResponse {
    bool status;
    std::string identity;
};

class TTNeighborsStub {
public:
    TTNeighborsStub() = default;
    virtual ~TTNeighborsStub() = default;
    TTNeighborsStub(const TTNeighborsStub&) = delete;
    TTNeighborsStub(TTNeighborsStub&&) = delete;
    TTNeighborsStub& operator=(const TTNeighborsStub&) = delete;
    TTNeighborsStub& operator=(TTNeighborsStub&&) = delete;
    virtual TTUniqueChatStub createChatStub(const std::string& ipAddressAndPort) const;
    virtual TTUniqueDiscoveryStub createDiscoveryStub(const std::string& ipAddressAndPort) const;
    virtual TTTellResponse sendTell(TTNeighborsChatStubIf& stub, const TTTellRequest& rhs) const;
    virtual TTNarrateResponse sendNarrate(TTNeighborsChatStubIf& stub, const TTNarrateRequest& rhs) const;
    virtual TTGreetResponse sendGreet(TTNeighborsDiscoveryStubIf& stub, const TTGreetRequest& rhs) const;
    virtual TTHeartbeatResponse sendHeartbeat(TTNeighborsDiscoveryStubIf& stub, const TTHeartbeatRequest& rhs) const;
};
