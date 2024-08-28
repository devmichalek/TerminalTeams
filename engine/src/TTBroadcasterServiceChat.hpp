#pragma once
#include <grpc++/grpc++.h>
#include "TTBroadcasterChat.hpp"
#include "TerminalTeams.grpc.pb.h"

class TTBroadcasterServiceChat final : public tt::NeighborsChat::Service {
public:
    TTBroadcasterServiceChat(TTBroadcasterChat& handler);
    ~TTBroadcasterServiceChat() = default;
    TTBroadcasterServiceChat(const TTBroadcasterServiceChat&) = delete;
    TTBroadcasterServiceChat(TTBroadcasterServiceChat&&) = delete;
    TTBroadcasterServiceChat& operator=(const TTBroadcasterServiceChat&) = delete;
    TTBroadcasterServiceChat& operator=(TTBroadcasterServiceChat&&) = delete;
    grpc::Status Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) override;
    grpc::Status Narrate(grpc::ServerContext* context, grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) override;
private:
    TTBroadcasterChat& mHandler;
};


