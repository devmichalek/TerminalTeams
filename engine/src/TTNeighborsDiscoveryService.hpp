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
private:
    TTNeighborsDiscovery& mHandler;
};
