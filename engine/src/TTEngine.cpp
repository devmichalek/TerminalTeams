#include "TTEngine.hpp"
#include "TTNeighborsServiceChat.hpp"
#include "TTNeighborsServiceDiscovery.hpp"
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
    LOG_INFO("Creating first contact and broadcasters...");
    {
        const auto i = settings.getInterface();
        mContacts->create(settings.getNickname(), settings.getIdentity(), i.getIpAddressAndPort());
        mContacts->select(0);
        mChat->create(0);
        mChat->select(0);
        mBroadcasterChat = std::make_unique<TTBroadcasterChat>(*mContacts, *mChat);
        mBroadcasterDiscovery = std::make_unique<TTBroadcasterDiscovery>(*mContacts, *mChat, settings.getInterface(), settings.getNeighbors());
    }
    LOG_INFO("Setting server thread...");
    {
        std::promise<void> serverPromise;
        mBlockers.push_back(serverPromise.get_future());
        mThreads.push_back(std::thread(&TTEngine::server, this, std::move(serverPromise)));
        mThreads.back().detach();
    }
    LOG_INFO("Setting broadcaster chat thread...");
    {
        std::promise<void> broadcasterPromise;
        mBlockers.push_back(broadcasterPromise.get_future());
        mThreads.push_back(std::thread(&TTEngine::chat, this, std::move(broadcasterPromise)));
        mThreads.back().detach();
    }
    LOG_INFO("Setting broadcaster discovery thread...");
    {
        std::promise<void> broadcasterPromise;
        mBlockers.push_back(broadcasterPromise.get_future());
        mThreads.push_back(std::thread(&TTEngine::discovery, this, std::move(broadcasterPromise)));
        mThreads.back().detach();
    }
    LOG_INFO("Successfully constructed!");
}

TTEngine::~TTEngine() {
    LOG_INFO("Destructing...");
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    LOG_INFO("Successfully destructed!");
}

void TTEngine::run() {
    LOG_INFO("Started main loop");
    while (!stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    LOG_INFO("Stopped main loop");
}

void TTEngine::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
    if (mServer) {
        mServer->Shutdown();
    }
    if (mContacts) {
        mContacts->stop();
    }
    if (mChat) {
        mChat->stop();
    }
    if (mTextBox) {
        mTextBox->stop();
    }
    if (mBroadcasterChat) {
        mBroadcasterChat->stop();
    }
    if (mBroadcasterDiscovery) {
        mBroadcasterDiscovery->stop();
    }
}

bool TTEngine::stopped() const {
    bool result = mStopped.load();
    result |= (mContacts && mContacts->stopped());
    result |= (mChat && mChat->stopped());
    result |= (mTextBox && mTextBox->stopped());
    result |= (mBroadcasterChat && mBroadcasterChat->stopped());
    result |= (mBroadcasterDiscovery && mBroadcasterDiscovery->stopped());
    return result;
}

void TTEngine::server(std::promise<void> promise) {
    // Setup
    LOG_INFO("Setting up server...");
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    LOG_INFO("Listening on {} without any authentication mechanism...", mBroadcasterDiscovery->getIpAddressAndPort());
    builder.AddListeningPort(mBroadcasterDiscovery->getIpAddressAndPort(), grpc::InsecureServerCredentials());
    LOG_INFO("Registering synchronous services...");
    TTNeighborsServiceChat neighborsServiceChat(*mBroadcasterChat);
    TTNeighborsServiceDiscovery neighborsServiceDiscovery(*mBroadcasterDiscovery);
    builder.RegisterService(&neighborsServiceChat);
    builder.RegisterService(&neighborsServiceDiscovery);
    LOG_INFO("Assembling the server and waiting for the server to shutdown...");
    mServer = std::unique_ptr<grpc::Server>(builder.BuildAndStart());
    // Start
    LOG_INFO("Started server loop");
    mServer->Wait();
    stop();
    promise.set_value();
    LOG_INFO("Completed server loop");
}

void TTEngine::chat(std::promise<void> promise) {
    LOG_INFO("Started chat loop");
    mBroadcasterChat->run();
    stop();
    promise.set_value();
    LOG_INFO("Completed chat loop");
}

void TTEngine::discovery(std::promise<void> promise) {
    LOG_INFO("Started discovery loop");
    mBroadcasterDiscovery->run(1);
    stop();
    promise.set_value();
    LOG_INFO("Completed discovery loop");
}

void TTEngine::mailbox(const std::string& message) {
    LOG_INFO("Received callback - message sent");
    if (mContacts && mChat && mBroadcasterChat) {
        std::scoped_lock lock(mExternalCallsMutex);
        if (!mContacts->current() || !mChat->current()) {
            LOG_ERROR("Received callback - failed to get current value!");
            stop();
        } else if (!mBroadcasterChat->handleSend(message)) {
            LOG_ERROR("Received callback - failed to sent message!");
            stop();
        }
    } else {
        LOG_ERROR("Received callback - failed to sent message to uninitialized instances!");
        stop();
    }
}

void TTEngine::switcher(size_t message) {
    LOG_INFO("Received callback - contacts switch");
    if (mContacts && mChat) {
        if (message < mContacts->size()) {
            std::scoped_lock lock(mExternalCallsMutex);
            bool result = mContacts->select(message);
            result &= mChat->select(message);
            if (!result) {
                LOG_ERROR("Received callback - failed to switch contact!");
                stop();
            }
        } else {
            LOG_WARNING("Received callback - attempt to switch to nonexisting contact!");
        }
    } else {
        LOG_ERROR("Received callback - failed to sent message to uninitialized instances!");
        stop();
    }
}