#pragma once
#include <gmock/gmock.h>
#include "TTNeighborsStub.hpp"

class TTNeighborsChatStubMock : public TTNeighborsChatStubIf {
public:
    MOCK_METHOD(::grpc::Status,
        Tell,
        (::grpc::ClientContext* context, const ::tt::TellRequest& request, ::tt::TellReply* response), (override));
    MOCK_METHOD(::grpc::ClientAsyncResponseReaderInterface< ::tt::TellReply>*,
        AsyncTellRaw,
        (::grpc::ClientContext* context, const ::tt::TellRequest& request, ::grpc::CompletionQueue* cq), (override));
    MOCK_METHOD(::grpc::ClientAsyncResponseReaderInterface< ::tt::TellReply>*,
        PrepareAsyncTellRaw,
        (::grpc::ClientContext* context, const ::tt::TellRequest& request, ::grpc::CompletionQueue* cq), (override));
    MOCK_METHOD(::grpc::ClientWriterInterface< ::tt::NarrateRequest>*,
        NarrateRaw,
        (::grpc::ClientContext* context, ::tt::NarrateReply* response), (override));
    MOCK_METHOD(::grpc::ClientAsyncWriterInterface< ::tt::NarrateRequest>*,
        AsyncNarrateRaw,
        (::grpc::ClientContext* context, ::tt::NarrateReply* response, ::grpc::CompletionQueue* cq, void* tag), (override));
    MOCK_METHOD(::grpc::ClientAsyncWriterInterface< ::tt::NarrateRequest>*,
        PrepareAsyncNarrateRaw,
        (::grpc::ClientContext* context, ::tt::NarrateReply* response, ::grpc::CompletionQueue* cq), (override));
};
