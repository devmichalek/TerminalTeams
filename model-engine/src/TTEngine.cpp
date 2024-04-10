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
    mContacts = std::make_unique<TTContactsHandler>(settings.getContactsSharedName(),
        std::bind(&TTEngine::contactsDataProduced, this),
        std::bind(&TTEngine::contactsDataConsumed, this));
    mChat = std::make_unique<TTChatHandler>(settings.getChatQueueName(),
        std::bind(&TTEngine::chatMessageSent, this),
        std::bind(&TTEngine::chatMessageReceived, this));
    mTextBox = std::make_unique<TTTextBoxHandler>(settings.getTextboxPipeName(),
        std::bind(&TTEngine::textBoxMessageSent, this),
        std::bind(&TTEngine::textBoxContactSwitch, this));
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(mIpAddressAndPort, grpc::InsecureServerCredentials());
    // Register synchronous services.
    for (auto &service : mServices) {
        builder.RegisterService(&service);
    }
    // Finally assemble the server and wait for the server to shutdown.
    mServer = std::make_unique<grpc::Server>(builder.BuildAndStart());
}

TTEngine::~TTEngine() {
    stop();
    mServerBlocker.wait();
}

void TTEngine::run() {
    mServer->Wait();
}

void TTEngine::stop() {
    mServer->Shutdown();
}

void TTEngine::contactsDataProduced() {
    // Nothing to be done
}

void TTEngine::contactsDataConsumed() {
    // Nothing to be done
}

void TTEngine::chatMessageSent() {
    // Nothing to be done
}

void TTEngine::chatMessageReceived() {
    // Nothing to be done
}

void TTEngine::textBoxMessageSent(std::string message) {

}

void TTEngine::textBoxContactSwitch(size_t message) {

}