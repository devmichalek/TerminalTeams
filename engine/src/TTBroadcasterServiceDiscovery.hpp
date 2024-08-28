#pragma once
#include <grpc++/grpc++.h>
#include "TTBroadcasterDiscovery.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTBroadcasterServiceDiscovery : public tt::NeighborsDiscovery::Service {
public:
    TTBroadcasterServiceDiscovery(TTBroadcasterDiscovery& handler);
    ~TTBroadcasterServiceDiscovery() = default;
    TTBroadcasterServiceDiscovery(const TTBroadcasterServiceDiscovery&) = delete;
    TTBroadcasterServiceDiscovery(TTBroadcasterServiceDiscovery&&) = delete;
    TTBroadcasterServiceDiscovery& operator=(const TTBroadcasterServiceDiscovery&) = delete;
    TTBroadcasterServiceDiscovery& operator=(TTBroadcasterServiceDiscovery&&) = delete;
    grpc::Status Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) override;
    grpc::Status Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) override;
private:
    TTBroadcasterDiscovery& mHandler;
};


