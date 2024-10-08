#include "TTNeighborsServiceDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighborsServiceDiscovery::TTNeighborsServiceDiscovery(TTBroadcasterDiscovery& handler) : mHandler(handler) {
    LOG_INFO("Successfully constructed!");
}

grpc::Status TTNeighborsServiceDiscovery::Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) {
    LOG_INFO("Handling greet...");
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
    TTGreetRequest message(request->nickname(), request->identity(), request->ipaddressandport());
    if (mHandler.handleGreet(message)) [[likely]] {
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
    LOG_INFO("Handling heartbeat...");
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
    const TTHeartbeatRequest message(request->identity());
    if (mHandler.handleHeartbeat(message)) [[likely]] {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}
