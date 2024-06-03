#include "TTEngine.hpp"
#include "TTNeighborsService.hpp"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

TTEngine::TTEngine(const TTEngineSettings& settings) {
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
    mNeighbors = std::make_unique<TTNeighbors>(settings.getInterface(), settings.getNeighbors(), *mContacts, *mChat);
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

void TTEngine::stop() {
    LOG_INFO("Forced stop...");
    mStopped.store(true);
    if (mContacts) {
        mContacts->stop();
    }
    if (mChat) {
        mChat->stop();
    }
    if (mTextBox) {
        mTextBox->stop();
    }
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
    builder.AddListeningPort(mNeighbors->getIpAddressAndPort(), grpc::InsecureServerCredentials());
    LOG_INFO("Registering synchronous services...");
    TTNeighborsService neighborsService(*mNeighbors, *mNeighbors);
    neighborsService.registerServices(builder);
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
    if (mChat) {
        std::scoped_lock<std::mutex> lock(mMailboxMutex);
        const auto now = std::chrono::system_clock::now();
        bool result = mChat->send(mContacts->current(), message, now);
        result &= mContacts->send(mContacts->current());
        if (!result) {
            LOG_ERROR("Received callback - failed to sent message!");
            stop();
        }
    }
}

void TTEngine::switcher(size_t message) {
    LOG_INFO("Received callback - contacts switch");
    if (mContacts) {
        if (message < mContacts->size()) {
            std::scoped_lock<std::mutex> lock(mSwitcherMutex);
            bool result = mContacts->select(message);
            result &= mChat->clear(message);
            if (!result) {
                LOG_ERROR("Received callback - failed to switch contact!");
                stop();
            }
        } else {
            LOG_WARNING("Received callback - attempt to switch to nonexisting contact!");
        }
    }
}