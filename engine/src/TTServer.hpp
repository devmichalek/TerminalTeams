#pragma once
#include "TTNeighborsServiceChat.hpp"
#include "TTNeighborsServiceDiscovery.hpp"
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

class TTServer {
public:
    TTServer(const std::string& ipAddressAndPort, TTNeighborsServiceChat& chat, TTNeighborsServiceDiscovery& discovery) {
        mStopped = false;
        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        grpc::ServerBuilder builder;
        builder.AddListeningPort(ipAddressAndPort, grpc::InsecureServerCredentials());
        builder.RegisterService(&chat);
        builder.RegisterService(&discovery);
        mGrpcServer = std::unique_ptr<grpc::Server>(builder.BuildAndStart());
    }
    virtual ~TTServer() = default;
    TTServer(const TTServer&) = default;
    TTServer(TTServer&&) = default;
    TTServer& operator=(const TTServer&) = default;
    TTServer& operator=(TTServer&&) = default;

    virtual void run() {
        mGrpcServer->Wait();
    }

    virtual void stop() {
        mGrpcServer->Shutdown();
        mStopped = true; // thread-safe as shutdown has internal mutex
    }

    [[nodiscard]] virtual bool stopped() const {
        return mStopped;
    }
protected:
    TTServer() = default;
private:
    bool mStopped;
    std::unique_ptr<grpc::Server> mGrpcServer;
};
