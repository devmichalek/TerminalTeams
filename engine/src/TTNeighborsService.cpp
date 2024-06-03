#include "TTNeighborsService.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighborsService::TTNeighborsService(TTNeighborsService::TTNeighborsChat& chatHandler, TTNeighborsService::TTNeighborsDiscovery& discoveryHandler) :
        mChatService(chatHandler), mDiscoveryService(discoveryHandler) {
    LOG_INFO("Successfully constructed!");
};

void TTNeighborsService::registerServices(grpc::ServerBuilder& builder) const {
    builder.RegisterService(&mChatService);
    builder.RegisterService(&mDiscoveryService);
    LOG_INFO("Successfully registered services!");
}

TTNeighborsService::Chat(TTNeighborsChat& handler) : mHandler(handler) {
    LOG_INFO("Successfully constructed!");
};

grpc::Status TTNeighborsService::Chat::Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status::INVALID_ARGUMENT;
    }
    TTNarrateMessage message;
    message.identity = request.identity();
    message.message = request.message();
    message.sequenceNumber = request.sequenceNumber();
    std::memcpy(&message.timestamp, request.timestamp().c_str(), request.timestamp().size());
    message.senderSide = true;
    if (mHandler.handleTell(messages)) {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status::UNKNOWN;
}

grpc::Status TTNeighborsService::Chat::Narrate(grpc::ServerContext* context, const grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status::INVALID_ARGUMENT;
    }
    TTNarrateMessages messages;
    tt::NarrateRequest request;
    while (request->Read(&request)) {
        if (request.side() == tt::Side::UNSPECIFIED) {
            LOG_ERROR("Side is unspecified!");
            return grpc::Status::OUT_OF_RANGE;
        }
        TTNarrateMessage message;
        message.identity = request.identity();
        message.message = request.message();
        message.sequenceNumber = request.sequenceNumber();
        std::memcpy(&message.timestamp, request.timestamp().c_str(), request.timestamp().size());
        message.senderSide = request.side() == tt::Side::SENDER;
        messages.push_back(message);
    }
    if (mHandler.handleNarrate(messages)) {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status::UNKNOWN;
}

TTNeighborsService::Discovery(TTNeighborsDiscovery& handler) : mHandler(handler) {
    LOG_INFO("Successfully constructed!");
}

grpc::Status TTNeighborsService::Discovery::Greet(grpc::ServerContext* context, const tt::GreetRequest* request, tt::GreetReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status::INVALID_ARGUMENT;
    }
    TTGreetMessage message;
    message.nickname = request.nickname();
    message.identity = request.identity();
    message.ipAddressAndPort = request.ipAddressAndPort();
    if (mHandler.handleGreet(message)) {
        reply->set_nickname(mHandler.getNickname());
        reply->set_identity(mHandler.getIdentity());
        reply->set_ipAddressAndPort(mHandler.getIpAddressAndPort());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status::UNKNOWN;
}

grpc::Status TTNeighborsService::Discovery::Heartbeat(grpc::ServerContext* context, const tt::HeartbeatRequest* request, tt::HeartbeatReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status::INVALID_ARGUMENT;
    }
    TTHeartbeatMessage message;
    message.identity = request.identity();
    if (mHandler.handleGreet(message)) {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status::UNKNOWN;
}
