#pragma once
#include <grpc++/grpc++.h>
#include "TTBroadcasterDiscovery.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTNeighborsServiceDiscovery : public tt::NeighborsDiscovery::Service {
public:
    TTNeighborsServiceDiscovery(TTBroadcasterDiscovery& handler);
    ~TTNeighborsServiceDiscovery() = default;
    TTNeighborsServiceDiscovery(const TTNeighborsServiceDiscovery&) = delete;
    TTNeighborsServiceDiscovery(TTNeighborsServiceDiscovery&&) = delete;
    TTNeighborsServiceDiscovery& operator=(const TTNeighborsServiceDiscovery&) = delete;
    TTNeighborsServiceDiscovery& operator=(TTNeighborsServiceDiscovery&&) = delete;
    grpc::Status Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) override;
    grpc::Status Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) override;
private:
    TTBroadcasterDiscovery& mHandler;
};


