#pragma once
#include <grpc++/grpc++.h>
#include "TTNeighborsChat.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTNeighborsChatService final : public tt::NeighborsChat::Service {
public:
    TTNeighborsChatService(TTNeighborsChat& handler);
    virtual ~TTNeighborsChatService();
    TTNeighborsChatService(const TTNeighborsChatService&) = delete;
    TTNeighborsChatService(TTNeighborsChatService&&) = delete;
    TTNeighborsChatService operator=(const TTNeighborsChatService&) = delete;
    TTNeighborsChatService operator=(TTNeighborsChatService&&) = delete;
private:
    TTNeighborsChat& mHandler;
};
