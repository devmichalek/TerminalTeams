#pragma once
#include "TTEngineSettings.hpp"

class TTEngine {
public:
    explicit TTEngine(const TTEngineSettings& settings);
    virtual ~TTEngine();
    TTEngine(const TTEngine&) = delete;
    TTEngine(TTEngine&&) = delete;
    TTEngine& operator=(const TTEngine&) = delete;
    TTEngine& operator=(TTEngine&&) = delete;
    // Main loop
    virtual void run();
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
private:
    // Server thread
    void server(std::promise<void> promise);
    // Broadcasters threads
    void chat(std::promise<void> promise);
    void discovery(std::promise<void> promise);
    // Callback functions
    void mailbox(const std::string& message);
    void switcher(size_t message);
    // Concurrent communication
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
    // Handlers, IPC communication
    std::unique_ptr<TTContactsHandler> mContacts;
    std::unique_ptr<TTChatHandler> mChat;
    std::unique_ptr<TTTextBoxHandler> mTextBox;
    std::mutex mExternalCallsMutex;
    std::mutex mLoopMutex;
    std::condition_variable mLoopCondition;
    // Node data
    std::unique_ptr<grpc::Server> mServer;
    std::unique_ptr<TTNeighborsStub> mNeighborsStub;
    std::unique_ptr<TTBroadcasterChat> mBroadcasterChat;
    std::unique_ptr<TTBroadcasterDiscovery> mBroadcasterDiscovery;
};
