#include "TTBroadcasterStub.hpp"
#include "TTDiagnosticsLogger.hpp"

UniqueChatStub TTBroadcasterStub::createChatStub(const std::string& ipAddressAndPort) {
    try {
        auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
        return NeighborsChat::NewStub(channel);
    } catch (...) {
        LOG_WARNING("Failed to create chat stub to {}!", ipAddressAndPort);
        return {};
    }
}

UniqueDiscoveryStub TTBroadcasterStub::createDiscoveryStub(const std::string& ipAddressAndPort) {
    try {
        auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
        return NeighborsDiscovery::NewStub(channel);
    } catch (...) {
        LOG_WARNING("Failed to create discovery stub to {}!", ipAddressAndPort);
        return {};
    }
}

TTTellResponse TTBroadcasterStub::sendTell(const UniqueChatStub& stub, const TTTellRequest& rhs) {
    try {
        TellRequest request;
        request.set_identity(rhs.identity);
        request.set_message(rhs.message);
        TellReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub->Tell(&context, request, &reply);
        if (status.ok()) {
            return {true};
        }
        LOG_ERROR("Error status received on send tell!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending tell!");
    }
    return {false};
}

TTNarrateResponse TTBroadcasterStub::sendNarrate(const UniqueChatStub& stub, const TTNarrateRequest& rhs) {
    try {
        grpc::ClientContext context;
        NarrateReply reply;
        std::unique_ptr<grpc::ClientWriter<NarrateRequest>> writer(stub->Narrate(&context, &reply));
        for (const auto &message : rhs.messages) {
            NarrateRequest request;
            request.set_identity(rhs.identity);
            request.set_message(message);
            if (!writer->Write(request)) {
                LOG_ERROR("Error occurred while sending narrate (broken stream)!");
                return {false};
            }
        }
        grpc::Status status = writer->Finish();
        if (status.ok()) {
            return {true};
        }
        LOG_ERROR("Error status received on send narrate!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending narrate!");
    }
    return {false};
}

TTGreetResponse TTBroadcasterStub::sendGreet(const UniqueDiscoveryStub& stub, const TTGreetRequest& rhs) {
    try {
        GreetRequest request;
        request.set_nickname(rhs.nickname);
        request.set_identity(rhs.identity);
        request.set_ipaddressandport(rhs.ipAddressAndPort);
        GreetReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub->Greet(&context, request, &reply);
        if (status.ok()) {
            TTGreetResponse result;
            result.status = true;
            result.nickname = reply.nickname();
            result.identity = reply.identity();
            result.ipAddressAndPort = reply.ipaddressandport();
            return result;
        }
        LOG_ERROR("Error status received on send greet!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending greet!");
    }
    return {};
}

TTHeartbeatResponse TTBroadcasterStub::sendHeartbeat(const UniqueDiscoveryStub& stub, const TTHeartbeatRequest& rhs) {
    try {
        HeartbeatRequest request;
        request.set_identity(rhs.identity);
        HeartbeatReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub->Heartbeat(&context, request, &reply);
        if (status.ok()) {
            TTHeartbeatResponse result;
            result.status = true;
            result.identity = reply.identity();
            return result;
        }
        LOG_ERROR("Error status received on send heartbeat!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending greet!");
    }
    return {};
}
