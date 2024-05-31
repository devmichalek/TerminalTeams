#include "TTNeighborsChatService.hpp"

TTNeighborsChatService::TTNeighborsChatService(TTNeighborsChat& handler) : mHandler(handler) {

}

TTNeighborsChatService::~TTNeighborsChatService() {

}

grpc::Status TTNeighborsChatService::Tell(grpc::ServerContext* context, const tt::TellRequest* request, tt::TellReply* reply) {
    if (!context) {
        return grpc::Status::INVALID_ARGUMENT;
    }
    return grpc::Status::OK;
}

grpc::Status TTNeighborsChatService::Narrate(grpc::ServerContext* context, const grpc::ServerReader<tt::NarrateRequest>* stream, tt::NarrateReply* reply) {
    if (!context) {
        return grpc::Status::INVALID_ARGUMENT;
    }
    tt::NarrateRequest request;
    while (request->Read(&request)) {

    }
    return grpc::Status::OK;
}
