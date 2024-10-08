#include "TTNeighborsServiceChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighborsServiceChat::TTNeighborsServiceChat(TTBroadcasterChat& handler) : mHandler(handler) {
    LOG_INFO("Successfully constructed!");
};

grpc::Status TTNeighborsServiceChat::Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) {
    LOG_INFO("Handling tell...");
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
    const TTTellRequest message(request->identity(), request->message());
    if (mHandler.handleReceive(message)) [[likely]] {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}

grpc::Status TTNeighborsServiceChat::Narrate(grpc::ServerContext* context, grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) {
    LOG_INFO("Handling narrate...");
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Context is null!");
    }
    if (!stream) {
        LOG_ERROR("Stream is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Stream is null!");
    }
    if (!reply) {
        LOG_ERROR("Reply is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Reply is null!");
    }
    TTNarrateRequest message;
    tt::NarrateRequest request;
    std::set<size_t> uniqueIds;
    std::hash<std::string> hasher;
    grpc::ServerReaderInterface<tt::NarrateRequest>* istream = stream;
    while (istream->Read(&request)) {
        message.identity = request.identity();
        message.messages.push_back(request.message());
        uniqueIds.insert(hasher(message.identity));
    }
    if (uniqueIds.size() != 1) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Wrong number of unique ids!");
    }
    if (mHandler.handleReceive(message)) [[likely]] {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}
