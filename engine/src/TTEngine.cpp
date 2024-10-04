#include "TTEngine.hpp"

TTEngine::TTEngine(const TTEngineSettings& settings) {
    LOG_INFO("Constructing...");
    using namespace std::placeholders;
    const auto& abstractFactory = settings.getAbstractFactory();
    LOG_INFO("Creating handlers...");
    mContacts = abstractFactory.createContactsHandler();
    mChat = abstractFactory.createChatHandler();
    mTextBox = abstractFactory.createTextBoxHandler(std::bind(&TTEngine::mailbox, this, _1), std::bind(&TTEngine::selection, this, _1));
    if (!mContacts || !mChat || !mTextBox) {
        throw std::runtime_error("TTEngine: Failed to create handlers!");
    }
    LOG_INFO("Creating first contact...");
    {
        const auto networkInterface = settings.getNetworkInterface();
        mContacts->create(settings.getNickname(), settings.getIdentity(), networkInterface.getIpAddressAndPort());
        mContacts->select(0);
        mChat->create(0);
        mChat->select(0);
    }
    LOG_INFO("Creating neighbors stub and broadcasters...");
    {
        const auto networkInterface = settings.getNetworkInterface();
        mNeighborsStub = abstractFactory.createNeighborsStub();
        if (!mNeighborsStub) {
            throw std::runtime_error("TTEngine: Failed to create neighbors stub!");
        }
        mBroadcasterChat = abstractFactory.createBroadcasterChat(*mContacts, *mChat, *mNeighborsStub, networkInterface);
        mBroadcasterDiscovery = abstractFactory.createBroadcasterDiscovery(*mContacts, *mChat, *mNeighborsStub, networkInterface, settings.getNeighbors());
        if (!mBroadcasterChat || !mBroadcasterDiscovery) {
            throw std::runtime_error("TTEngine: Failed to create broadcasters!");
        }
    }
    LOG_INFO("Creating gRPC server...");
    {
        const auto networkInterface = settings.getNetworkInterface();
        mServiceChat = abstractFactory.createNeighborsServiceChat(*mBroadcasterChat);
        mServiceDiscovery = abstractFactory.createNeighborsServiceDiscovery(*mBroadcasterDiscovery);
        if (!mServiceChat || !mServiceDiscovery) {
            throw std::runtime_error("TTEngine: Failed to create services!");
        }
        mServer = abstractFactory.createServer(networkInterface.getIpAddressAndPort(), *mServiceChat, *mServiceDiscovery);
        if (!mServer) {
            throw std::runtime_error("TTEngine: Failed to create gRPC server!");
        }
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
    while (true) {
        bool stopped = isStopped();
        stopped |= (!mServer || mServer->isStopped());
        stopped |= (!mContacts || mContacts->isStopped());
        stopped |= (!mChat || mChat->isStopped());
        stopped |= (!mTextBox || mTextBox->isStopped());
        stopped |= (!mBroadcasterChat || mBroadcasterChat->isStopped());
        stopped |= (!mBroadcasterDiscovery || mBroadcasterDiscovery->isStopped());
        if (stopped) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
    stop();
    LOG_INFO("Stopped main loop");
}

void TTEngine::server(std::promise<void> promise) {
    LOG_INFO("Started server loop");
    mServer->run();
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
    mBroadcasterDiscovery->run();
    stop();
    promise.set_value();
    LOG_INFO("Completed discovery loop");
}

void TTEngine::mailbox(const std::string& message) {
    LOG_INFO("Received callback - message sent");
    std::scoped_lock lock(mExternalCallsMutex);
    if (!mBroadcasterChat->handleSend(message)) [[unlikely]] {
        LOG_ERROR("Received callback - failed to sent message!");
        stop();
    }
}

void TTEngine::selection(size_t message) {
    LOG_INFO("Received callback - contacts selection");
    if (message < mContacts->size()) {
        std::scoped_lock lock(mExternalCallsMutex);
        bool result = mContacts->select(message);
        result &= mChat->select(message);
        if (!result) [[unlikely]] {
            LOG_ERROR("Received callback - failed to select contact!");
            stop();
        }
    } else {
        LOG_WARNING("Received callback - attempt to select to nonexisting contact!");
    }
}

void TTEngine::onStop() {
    LOG_WARNING("Forced internal stop...");
    if (mServer) {
        mServer->stop();
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
