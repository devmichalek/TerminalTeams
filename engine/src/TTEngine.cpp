#include "TTEngine.hpp"

TTEngine::TTEngine(const TTEngineSettings& settings) {
    LOG_INFO("Constructing...");
    mStopped.store(false);
    using namespace std::placeholders;
    const auto& abstractFactory = settings.getAbstractFactory();
    mContacts = abstractFactory.createContactsHandler();
    mChat = abstractFactory.createChatHandler();
    mTextBox = abstractFactory.createTextBoxHandler(std::bind(&TTEngine::mailbox, this, _1), std::bind(&TTEngine::switcher, this, _1));
    LOG_INFO("Creating first contact and broadcasters...");
    {
        const auto networkInterface = settings.getNetworkInterface();
        mContacts->create(settings.getNickname(), settings.getIdentity(), networkInterface.getIpAddressAndPort());
        mContacts->select(0);
        mChat->create(0);
        mChat->select(0);
        mNeighborsStub = abstractFactory.createNeighborsStub();
        mBroadcasterChat = abstractFactory.createBroadcasterChat(*mContacts, *mChat, *mNeighborsStub, networkInterface);
        mBroadcasterDiscovery = abstractFactory.createBroadcasterDiscovery(*mContacts, *mChat, *mNeighborsStub, networkInterface, settings.getNeighbors());
    }
    LOG_INFO("Setting server thread...");
    {
        const auto networkInterface = settings.getNetworkInterface();
        auto neighborsServiceChat = abstractFactory.createNeighborsServiceChat(*mBroadcasterChat);
        auto neighborsServiceDiscovery = abstractFactory.createNeighborsServiceDiscovery(*mBroadcasterDiscovery);
        mServer = abstractFactory.createServer(networkInterface.getIpAddressAndPort(), *neighborsServiceChat, *neighborsServiceDiscovery);
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
    size_t counter = 0;
    while (!stopped()) {
        LOG_INFO("Loop counter: {}", counter++);
        std::unique_lock<std::mutex> lock(mLoopMutex);
        const bool predicate = mLoopCondition.wait_for(lock, std::chrono::milliseconds(5000), [this]() {
            return stopped();
        });
        if (predicate) {
            break;
        }
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
    mBroadcasterDiscovery->run();
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