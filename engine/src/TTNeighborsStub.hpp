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
    TTGreetRequest(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) :
        nickname(nickname), identity(identity), ipAddressAndPort(ipAddressAndPort) {}
    bool operator==(const TTGreetRequest& rhs) const {
        return nickname == rhs.nickname && identity == rhs.identity && ipAddressAndPort == rhs.ipAddressAndPort;
    }
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTGreetResponse {
    TTGreetResponse(bool status, const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) :
        status(status), nickname(nickname), identity(identity), ipAddressAndPort(ipAddressAndPort) {}
    bool status;
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTHeartbeatRequest {
    TTHeartbeatRequest(const std::string& identity) : identity(identity) {}
    bool operator==(const TTHeartbeatRequest& rhs) const {
        return identity == rhs.identity;
    }
    std::string identity;
};

struct TTHeartbeatResponse {
    TTHeartbeatResponse(bool status, const std::string& identity) : status(status), identity(identity) {}
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
