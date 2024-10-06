#include "TTNeighborsStub.hpp"
#include "TTDiagnosticsLogger.hpp"

TTUniqueChatStub TTNeighborsStub::createChatStub(const std::string& ipAddressAndPort) const {
    try {
        LOG_INFO("Creating chat stub to {}!", ipAddressAndPort);
        auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
        return tt::NeighborsChat::NewStub(channel);
    } catch (...) {
        LOG_WARNING("Failed to create chat stub to {}!", ipAddressAndPort);
        return {};
    }
}

TTUniqueDiscoveryStub TTNeighborsStub::createDiscoveryStub(const std::string& ipAddressAndPort) const {
    try {
        LOG_INFO("Creating discovery stub to {}!", ipAddressAndPort);
        auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
        return tt::NeighborsDiscovery::NewStub(channel);
    } catch (...) {
        LOG_WARNING("Failed to create discovery stub to {}!", ipAddressAndPort);
        return {};
    }
}

TTTellResponse TTNeighborsStub::sendTell(TTNeighborsChatStubIf& stub, const TTTellRequest& rhs) const {
    try {
        LOG_INFO("Sending tell...");
        tt::TellRequest request;
        request.set_identity(rhs.identity);
        request.set_message(rhs.message);
        tt::TellReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub.Tell(&context, request, &reply);
        if (status.ok()) [[likely]] {
            return {true};
        }
        LOG_ERROR("Error status received on send tell!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending tell!");
    }
    return {false};
}

TTNarrateResponse TTNeighborsStub::sendNarrate(TTNeighborsChatStubIf& stub, const TTNarrateRequest& rhs) const {
    try {
        LOG_INFO("Sending narrate...");
        grpc::ClientContext context;
        tt::NarrateReply reply;
        std::unique_ptr<grpc::ClientWriterInterface<tt::NarrateRequest>> writer(stub.Narrate(&context, &reply));
        if (!writer) {
            LOG_ERROR("Failed to create writer on send narrate!");
            return {false};
        }
        for (const auto &message : rhs.messages) {
            tt::NarrateRequest request;
            request.set_identity(rhs.identity);
            request.set_message(message);
            if (!writer->Write(request)) {
                LOG_ERROR("Error occurred while sending narrate (broken stream)!");
                return {false};
            }
        }
        writer->WritesDone();
        grpc::Status status = writer->Finish();
        if (status.ok()) [[likely]] {
            return {true};
        }
        LOG_ERROR("Error status received on send narrate!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending narrate!");
    }
    return {false};
}

TTGreetResponse TTNeighborsStub::sendGreet(TTNeighborsDiscoveryStubIf& stub, const TTGreetRequest& rhs) const {
    try {
        LOG_INFO("Sending greet...");
        tt::GreetRequest request;
        request.set_nickname(rhs.nickname);
        request.set_identity(rhs.identity);
        request.set_ipaddressandport(rhs.ipAddressAndPort);
        tt::GreetReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub.Greet(&context, request, &reply);
        if (status.ok()) [[likely]] {
            return TTGreetResponse(true, reply.nickname(), reply.identity(), reply.ipaddressandport());
        }
        LOG_ERROR("Error status received on send greet!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending greet!");
    }
    return {{}, {}, {}, {}};
}

TTHeartbeatResponse TTNeighborsStub::sendHeartbeat(TTNeighborsDiscoveryStubIf& stub, const TTHeartbeatRequest& rhs) const {
    try {
        LOG_INFO("Sending heartbeat...");
        tt::HeartbeatRequest request;
        request.set_identity(rhs.identity);
        tt::HeartbeatReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub.Heartbeat(&context, request, &reply);
        if (status.ok()) [[likely]] {
            return TTHeartbeatResponse(true, reply.identity());
        }
        LOG_ERROR("Error status received on send heartbeat!");
    } catch (...) {
        LOG_ERROR("Exception occurred while sending greet!");
    }
    return {{}, {}};
}
