#pragma once
#include <gmock/gmock.h>
#include "TTNeighborsServiceDiscovery.hpp"
#include "TTBroadcasterDiscoveryMock.hpp"

class TTNeighborsServiceDiscoveryMock : public TTNeighborsServiceDiscovery {
public:
    TTNeighborsServiceDiscoveryMock() : TTNeighborsServiceDiscovery(mBroadcasterDiscovery) {}
    MOCK_METHOD(grpc::Status, Greet, (grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply), (override));
    MOCK_METHOD(grpc::Status, Heartbeat, (grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply), (override));
private:
    TTBroadcasterDiscoveryMock mBroadcasterDiscovery;
};
