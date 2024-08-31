#pragma once
#include "TerminalTeams.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using tt::NeighborsChat;
using tt::TellRequest;
using tt::TellReply;
using tt::NarrateRequest;
using tt::NarrateReply;
using tt::NeighborsDiscovery;
using tt::GreetRequest;
using tt::GreetReply;
using tt::HeartbeatRequest;
using tt::HeartbeatReply;

using TTNeighborsChatStubIf = NeighborsChat::StubInterface;
using TTNeighborsDiscoveryStubIf = NeighborsDiscovery::StubInterface;
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
