#include "TTNeighborsServiceDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighborsServiceDiscovery::TTNeighborsServiceDiscovery(TTBroadcasterDiscovery& handler) : mHandler(handler) {
    LOG_INFO("Successfully constructed!");
}

grpc::Status TTNeighborsServiceDiscovery::Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Context is null!");
    }
    if (!request) {
        LOG_ERROR("Request is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Request is null!");
    }
    if (!reply) {
        LOG_ERROR("Reply is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Reply is null!");
    }
    TTGreetRequest message;
    message.nickname = request->nickname();
    message.identity = request->identity();
    message.ipAddressAndPort = request->ipaddressandport();
    if (mHandler.handleGreet(message)) {
        reply->set_nickname(mHandler.getNickname());
        reply->set_identity(mHandler.getIdentity());
        reply->set_ipaddressandport(mHandler.getIpAddressAndPort());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}

grpc::Status TTNeighborsServiceDiscovery::Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Context is null!");
    }
    if (!request) {
        LOG_ERROR("Request is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Request is null!");
    }
    if (!reply) {
        LOG_ERROR("Reply is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Reply is null!");
    }
    TTHeartbeatRequest message;
    message.identity = request->identity();
    if (mHandler.handleHeartbeat(message)) {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}
