#include "TTEngine.hpp"
#include "TTNeighborsChatService.hpp"
#include "TTNeighborsDiscoveryService.hpp"
#include "absl/strings/str_format.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

TTEngine::TTEngine(const TTEngineSettings& settings) :
        mInterface(settings.getInterface()),
        mIpAddressAndPort(absl::StrFormat("%s:%d", settings.getIpAddress().c_str(), settings.getPort())),
        mNeighbors(settings.getNeighbors()) {
    LOG_INFO("Constructing...");
    using namespace std::placeholders;
    mContacts = std::make_unique<TTContactsHandler>(settings.getContactsSettings());
    mChat = std::make_unique<TTChatHandler>(settings.getChatSettings());
    mTextBox = std::make_unique<TTTextBoxHandler>(settings.getTextBoxSettings(),
        std::bind(&TTEngine::mailbox, this, _1),
        std::bind(&TTEngine::switcher, this, _1));
    // Set server thread
    {
        std::promise<void> serverPromise;
        mBlockers.push_back(serverPromise.get_future());
        mThreads.push_back(std::thread(&TTEngine::server, this, std::move(serverPromise)));
        mThreads.back().detach();
    }
    LOG_INFO("Successfully constructed!");
}

TTEngine::~TTEngine() {
    LOG_INFO("Destructing...");
    stop();
    if (mServer) {
        LOG_INFO("Stopping server...");
        mServer->Shutdown();
    } else {
        LOG_WARNING("Failed stop already stopped server!");
    }
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    LOG_INFO("Successfully destructed!");
}

void TTEngine::run() {
    LOG_INFO("Started main loop");

    LOG_INFO("Completed main loop");
}

void TTEngine::stop() {
    LOG_INFO("Forced stop...");
    mStopped.store(true);
}

bool TTEngine::stopped() const {
    return mStopped.load();
}

void TTEngine::server(std::promise<void> promise) {
    // Setup
    LOG_INFO("Setting up server...");
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    LOG_INFO("Listening on the given address without any authentication mechanism...");
    builder.AddListeningPort(mIpAddressAndPort, grpc::InsecureServerCredentials());
    LOG_INFO("Registering synchronous services...");
    TTNeighborsChatService neighborsChatService(mNeighborsChat);
    TTNeighborsDiscoveryService neighborsDiscoveryService(mNeighborsDiscovery);
    builder.RegisterService(&neighborsChatService);
    builder.RegisterService(&neighborsDiscoveryService);
    LOG_INFO("Assembling the server and waiting for the server to shutdown...");
    mServer = std::unique_ptr<grpc::Server>(builder.BuildAndStart());
    // Start
    LOG_INFO("Started server loop");
    mServer->Wait();
    stop();
    promise.set_value();
    LOG_INFO("Completed server loop");
}

void TTEngine::mailbox(std::string message) {
    LOG_INFO("Received callback - message sent");

}

void TTEngine::switcher(size_t message) {
    LOG_INFO("Received callback - contacts switch");

}