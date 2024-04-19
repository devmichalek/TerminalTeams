#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"


#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "TerminalTeams.grpc.pb.h"
#include "TTConfig.hpp"

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public tt::Greeter::Service {
  grpc::Status SayHello(grpc::ServerContext* context, const tt::HelloRequest* request,
                  tt::HelloReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return grpc::Status::OK;
  }
};


void TTServer::run() {
    
}