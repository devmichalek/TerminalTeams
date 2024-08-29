#pragma once
#include <gmock/gmock.h>
#include "TTNeighborsStub.hpp"

class TTNeighborsDiscoveryStubMock : public TTNeighborsDiscoveryStubIf {
public:
    MOCK_METHOD(::grpc::Status,
        Greet,
        (::grpc::ClientContext* context, const ::tt::GreetRequest& request, ::tt::GreetReply* response), (override));
    MOCK_METHOD(::grpc::Status,
        Heartbeat,
        (::grpc::ClientContext* context, const ::tt::HeartbeatRequest& request, ::tt::HeartbeatReply* response), (override));
    MOCK_METHOD(::grpc::ClientAsyncResponseReaderInterface< ::tt::GreetReply>*,
        AsyncGreetRaw,
        (::grpc::ClientContext* context, const ::tt::GreetRequest& request, ::grpc::CompletionQueue* cq), (override));
    MOCK_METHOD(::grpc::ClientAsyncResponseReaderInterface< ::tt::GreetReply>*,
        PrepareAsyncGreetRaw,
        (::grpc::ClientContext* context, const ::tt::GreetRequest& request, ::grpc::CompletionQueue* cq), (override));
    MOCK_METHOD(::grpc::ClientAsyncResponseReaderInterface< ::tt::HeartbeatReply>*,
        AsyncHeartbeatRaw,
        (::grpc::ClientContext* context, const ::tt::HeartbeatRequest& request, ::grpc::CompletionQueue* cq), (override));
    MOCK_METHOD(::grpc::ClientAsyncResponseReaderInterface< ::tt::HeartbeatReply>*,
        PrepareAsyncHeartbeatRaw,
        (::grpc::ClientContext* context, const ::tt::HeartbeatRequest& request, ::grpc::CompletionQueue* cq), (override));
};
