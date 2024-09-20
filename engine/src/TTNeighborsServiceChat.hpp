#pragma once
#include <grpc++/grpc++.h>
#include "TTBroadcasterChat.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTNeighborsServiceChat : public tt::NeighborsChat::Service {
public:
    TTNeighborsServiceChat(TTBroadcasterChat& handler);
    ~TTNeighborsServiceChat() = default;
    TTNeighborsServiceChat(const TTNeighborsServiceChat&) = delete;
    TTNeighborsServiceChat(TTNeighborsServiceChat&&) = delete;
    TTNeighborsServiceChat& operator=(const TTNeighborsServiceChat&) = delete;
    TTNeighborsServiceChat& operator=(TTNeighborsServiceChat&&) = delete;
    grpc::Status Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) override;
    grpc::Status Narrate(grpc::ServerContext* context, grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) override;
private:
    TTBroadcasterChat& mHandler;
};
