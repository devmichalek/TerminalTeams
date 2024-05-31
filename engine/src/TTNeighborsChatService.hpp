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
    grpc::Status Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) override;
    grpc::Status Narrate(grpc::ServerContext* context, const grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) override;
private:
    TTNeighborsChat& mHandler;
};
