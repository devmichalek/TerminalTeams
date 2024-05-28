#include "TTEngine.hpp"
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTTextBoxHandler.hpp"
#include "absl/strings/str_format.h"

TTEngine::TTEngine(const TTEngineSettings& settings,
    std::vector<tt::Greeter::Service> services) :
        mInterface(settings.getInterface()),
        mIpAddress(settings.getIpAddress()),
        mIpAddressAndPort(absl::StrFormat("%s:%d", settings.getIpAddress().c_str(), settings.getPort())),
        mNeighbors(settings.getNeighbors()),
        mServices(services) {
    LOG_INFO("Constructing...");
    mContacts = std::make_unique<TTContactsHandler>(settings.getContactsSettings());
    mChat = std::make_unique<TTChatHandler>(settings.getChatSettings());
    mTextBox = std::make_unique<TTTextBoxHandler>(settings.getTextBoxSettings(),
        std::bind(&TTEngine::textBoxMessageSent, this),
        std::bind(&TTEngine::textBoxContactSwitch, this));
    // Set server thread
    {
        std::promise<void> serverPromise;
        mBlockers.push_back(serverPromise.get_future());
        mThreads.push_back(std::thread(&TTTextBox::server, this, std::move(serverPromise)));
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

bool TTTextBox::stopped() const {
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
    for (auto &service : mServices) {
        builder.RegisterService(&service);
    }
    LOG_INFO("Assembling the server and waiting for the server to shutdown...");
    mServer = std::make_unique<grpc::Server>(builder.BuildAndStart());
    // Start
    LOG_INFO("Started server loop");
    mServer->Wait();
    stop();
    promise.set_value();
    LOG_INFO("Completed server loop");
}

void TTEngine::textBoxMessageSent(std::string message) {
    LOG_INFO("Received callback - message sent");

}

void TTEngine::textBoxContactSwitch(size_t message) {
    LOG_INFO("Received callback - contacts switch");

}