#include "TTNeighborsServiceChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighborsServiceChat::TTNeighborsServiceChat(TTBroadcasterChatIf& handler) : mHandler(handler) {
    LOG_INFO("Successfully constructed!");
};

grpc::Status TTNeighborsServiceChat::Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Context is null!");
    }
    TTNarrateMessage message;
    message.identity = request->identity();
    message.message = request->message();
    message.sequenceNumber = request->sequencenumber();
    std::memcpy(&message.timestamp, request->timestamp().c_str(), request->timestamp().size());
    message.senderSide = true;
    if (mHandler.handleTell(message)) {
        reply->set_identity(mHandler.getIdentity());
        LOG_INFO("Successfully handled request!");
        return grpc::Status::OK;
    }
    LOG_ERROR("Failed to handle request!");
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}

grpc::Status TTNeighborsServiceChat::Narrate(grpc::ServerContext* context, grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) {
    if (!context) {
        LOG_ERROR("Context is null!");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Context is null!");
    }
    TTNarrateMessages messages;
    tt::NarrateRequest request;
    while (stream->Read(&request)) {
        if (request.side() == tt::Side::UNSPECIFIED) {
            LOG_ERROR("Side is unspecified!");
            return grpc::Status(grpc::StatusCode::OUT_OF_RANGE, "Side is unspecified!");
        }
        TTNarrateMessage message;
        message.identity = request.identity();
        message.message = request.message();
        message.sequenceNumber = request.sequencenumber();
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
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Failed to handle request!");
}
