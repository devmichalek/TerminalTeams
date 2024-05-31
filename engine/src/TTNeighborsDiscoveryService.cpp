#include "TTNeighborsDiscoveryService.hpp"

TTNeighborsDiscoveryService::TTNeighborsDiscoveryService(TTNeighborsDiscovery& handler) : mHandler(handler) {

}

TTNeighborsDiscoveryService::~TTNeighborsDiscoveryService() {

}

grpc::Status Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) {
    if (!context) {
        return grpc::Status::INVALID_ARGUMENT;
    }
    return grpc::Status::OK;
}

grpc::Status Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) {
    if (!context) {
        return grpc::Status::INVALID_ARGUMENT;
    }
    return grpc::Status::OK;
}
