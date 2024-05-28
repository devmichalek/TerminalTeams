#pragma once
#include "TTEngineSettings.hpp"
#include <memory>
#include <string>
#include <thread>
#include <future>

class TTEngine {
public:
    explicit TTEngine(const TTEngineSettings& settings,
        std::vector<tt::Greeter::Service> services);
    virtual ~TTEngine();
    TTTextBox(const TTTextBox&) = delete;
    TTTextBox(const TTTextBox&&) = delete;
    TTTextBox operator=(const TTTextBox&) = delete;
    TTTextBox operator=(const TTTextBox&&) = delete;
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
    void textBoxMessageSent(std::string message);
    void textBoxContactSwitch(size_t message);
    // Handlers, IPC communication
    std::unique_ptr<TTContactsHandler> mContacts;
    std::unique_ptr<TTChatHandler> mChat;
    std::unique_ptr<TTTextBoxHandler> mTextBox;
    // Node data
    std::string mInterface;
    std::string mIpAddressAndPort;
    std::vector<std::string> mNeighbors;
    // Server
    std::vector<tt::Greeter::Service> mServices;
    std::unique_ptr<grpc::Server> mServer;
    // Concurrent communication
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
}