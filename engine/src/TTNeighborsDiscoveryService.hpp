#pragma once
#include <grpc++/grpc++.h>
#include "TTNeighborsDiscovery.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTNeighborsDiscoveryService : public tt::NeighborsDiscovery::Service {
public:
    TTNeighborsDiscoveryService(TTNeighborsDiscovery& handler);
    virtual ~TTNeighborsDiscoveryService();
    TTNeighborsDiscoveryService(const TTNeighborsDiscoveryService&) = delete;
    TTNeighborsDiscoveryService(TTNeighborsDiscoveryService&&) = delete;
    TTNeighborsDiscoveryService operator=(const TTNeighborsDiscoveryService&) = delete;
    TTNeighborsDiscoveryService operator=(TTNeighborsDiscoveryService&&) = delete;
    grpc::Status Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) override;
    grpc::Status Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) override;
private:
    TTNeighborsDiscovery& mHandler;
};
