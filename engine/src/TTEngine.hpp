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
    ~TTEngine();
    void run();
    void stop();
private:
    // Callback functions
    void contactsDataProduced();
    void contactsDataConsumed();
    void chatMessageSent();
    void chatMessageReceived();
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
    std::thread mServerThread;
    std::future<void> mServerBlocker;
}