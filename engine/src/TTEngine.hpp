#pragma once
#include "TTEngineSettings.hpp"
#include "TTNeighbors.hpp"
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTTextBoxHandler.hpp"
#include <grpcpp/grpcpp.h>

class TTEngine {
public:
    explicit TTEngine(const TTEngineSettings& settings);
    virtual ~TTEngine();
    TTEngine(const TTEngine&) = delete;
    TTEngine(TTEngine&&) = delete;
    TTEngine& operator=(const TTEngine&) = delete;
    TTEngine& operator=(TTEngine&&) = delete;
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
private:
    // Server thread
    void server(std::promise<void> promise);
    // Callback functions
    void mailbox(std::string message);
    void switcher(size_t message);
    // Handlers, IPC communication
    std::unique_ptr<TTContactsHandler> mContacts;
    std::unique_ptr<TTChatHandler> mChat;
    std::unique_ptr<TTTextBoxHandler> mTextBox;
    std::mutex mMailboxMutex;
    std::mutex mSwitcherMutex;
    // Node data
    std::unique_ptr<grpc::Server> mServer;
    std::unique_ptr<TTNeighbors> mNeighbors;
    // Concurrent communication
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};
