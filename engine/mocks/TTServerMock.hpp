#pragma once
#include <gmock/gmock.h>
#include <grpcpp/grpcpp.h>

class TTServerMock : public grpc::Server {
public:
    TTServerMock() : grpc::Server(&mChannelArgs, nullptr, 0, 0, 0, {}) {}
private:
    grpc::ChannelArguments mChannelArgs;
};