#pragma once
#include <grpc++/grpc++.h>
#include "TTNeighborsChat.hpp"
#include "TTNeighborsDiscovery.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTNeighborsService {
public:
    TTNeighborsService(TTNeighborsChat& chatHandler, TTNeighborsDiscovery& discoveryHandler);
    ~TTNeighborsService() = default;
    TTNeighborsService(const TTNeighborsService&) = delete;
    TTNeighborsService(TTNeighborsService&&) = delete;
    TTNeighborsService& operator=(const TTNeighborsService&) = delete;
    TTNeighborsService& operator=(TTNeighborsService&&) = delete;
    void registerServices(grpc::ServerBuilder& builder);
private:
    class Chat final : public tt::NeighborsChat::Service {
    public:
        Chat(TTNeighborsChat& handler);
        ~Chat() = default;
        Chat(const Chat&) = delete;
        Chat(Chat&&) = delete;
        Chat& operator=(const Chat&) = delete;
        Chat& operator=(Chat&&) = delete;
        grpc::Status Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) override;
        grpc::Status Narrate(grpc::ServerContext* context, grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) override;
    private:
        TTNeighborsChat& mHandler;
    };

    class Discovery : public tt::NeighborsDiscovery::Service {
    public:
        Discovery(TTNeighborsDiscovery& handler);
        ~Discovery() = default;
        Discovery(const Discovery&) = delete;
        Discovery(Discovery&&) = delete;
        Discovery& operator=(const Discovery&) = delete;
        Discovery& operator=(Discovery&&) = delete;
        grpc::Status Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) override;
        grpc::Status Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) override;
    private:
        TTNeighborsDiscovery& mHandler;
    };

    Chat mChatService;
    Discovery mDiscoveryService;
};


