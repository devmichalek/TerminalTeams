#pragma once
#include "TTEngineSettings.hpp"
#include "TTNeighborsChat.hpp"
#include "TTNeighborsDiscovery.hpp"
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
    TTEngine operator=(const TTEngine&) = delete;
    TTEngine operator=(TTEngine&&) = delete;
    // Starts application
    virtual void run();
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
    // Node data
    std::string mInterface;
    std::string mIpAddressAndPort;
    std::vector<std::string> mNeighbors;
    // Server
    std::unique_ptr<grpc::Server> mServer;
    TTNeighborsChat mNeighborsChat;
    TTNeighborsDiscovery mNeighborsDiscovery;
    // Concurrent communication
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};
