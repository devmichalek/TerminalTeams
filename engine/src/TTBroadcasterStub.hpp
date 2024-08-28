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

using UniqueChatStub = std::unique_ptr<NeighborsChat::Stub>;
using UniqueDiscoveryStub = std::unique_ptr<NeighborsDiscovery::Stub>;

struct TTTellRequest {
    std::string identity;
    std::string message;
};

struct TTTellResponse {
    bool status;
};

struct TTNarrateRequest {
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

class TTBroadcasterStub {
public:
    TTBroadcasterStub() = default;
    virtual ~TTBroadcasterStub() = default;
    TTBroadcasterStub(const TTBroadcasterStub&) = delete;
    TTBroadcasterStub(TTBroadcasterStub&&) = delete;
    TTBroadcasterStub& operator=(const TTBroadcasterStub&) = delete;
    TTBroadcasterStub& operator=(TTBroadcasterStub&&) = delete;
    virtual UniqueChatStub createChatStub(const std::string& ipAddressAndPort);
    virtual UniqueDiscoveryStub createDiscoveryStub(const std::string& ipAddressAndPort);
    virtual TTTellResponse sendTell(const UniqueChatStub& stub, const TTTellRequest& rhs);
    virtual TTNarrateResponse sendNarrate(const UniqueChatStub& stub, const TTNarrateRequest& rhs);
    virtual TTGreetResponse sendGreet(const UniqueDiscoveryStub& stub, const TTGreetRequest& rhs);
    virtual TTHeartbeatResponse sendHeartbeat(const UniqueDiscoveryStub& stub, const TTHeartbeatRequest& rhs);
};
