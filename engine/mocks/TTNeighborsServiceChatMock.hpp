#pragma once
#include <gmock/gmock.h>
#include "TTNeighborsServiceChat.hpp"
#include "TTBroadcasterChatMock.hpp"

class TTNeighborsServiceChatMock : public TTNeighborsServiceChat {
public:
    TTNeighborsServiceChatMock() : TTNeighborsServiceChat(mBroadcasterChat) {}
    MOCK_METHOD(grpc::Status, Tell, (grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply), (override));
    MOCK_METHOD(grpc::Status, Narrate, (grpc::ServerContext* context, grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply), (override));
private:
    TTBroadcasterChatMock mBroadcasterChat;
};
