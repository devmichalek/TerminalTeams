#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "TerminalTeams.grpc.pb.h"
#include "TTConfig.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using tt::Greeter;
using tt::HelloReply;
using tt::HelloRequest;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

TTServer::TTServer(uint32_t ipAddress, uint16_t port) {
	mIpAddressAndPort = absl::StrFormat("0.0.0.0:%d", port);
}

void TTServer::run() {
	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(mIpAddressAndPort, grpc::InsecureServerCredentials());
	// Register synchronous services.
	GreeterServiceImpl service;
	builder.RegisterService(&service);
	// Finally assemble the server and wait for the server to shutdown.
	std::unique_ptr<Server> server(builder.BuildAndStart());
	server->Wait();
}