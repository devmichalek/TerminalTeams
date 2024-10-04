#pragma once
#include "TTEngineSettings.hpp"
#include "TTUtilsStopable.hpp"

class TTEngine : public TTUtilsStopable {
public:
    explicit TTEngine(const TTEngineSettings& settings);
    virtual ~TTEngine();
    TTEngine(const TTEngine&) = delete;
    TTEngine(TTEngine&&) = delete;
    TTEngine& operator=(const TTEngine&) = delete;
    TTEngine& operator=(TTEngine&&) = delete;
    // Main loop
    virtual void run();
private:
    // Server thread
    void server(std::promise<void> promise);
    // Broadcaster chat thread
    void chat(std::promise<void> promise);
    // Broadcaster discovery thread
    void discovery(std::promise<void> promise);
    // Callback mailbox function
    void mailbox(const std::string& message);
    // Callback selection function (contacts selection)
    void selection(size_t message);
    // Stops application (internal function)
    virtual void onStop() override;
    // Concurrent communication
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
    // Handlers, IPC communication
    std::unique_ptr<TTContactsHandler> mContacts;
    std::unique_ptr<TTChatHandler> mChat;
    std::unique_ptr<TTTextBoxHandler> mTextBox;
    std::mutex mExternalCallsMutex;
    // Node data
    std::unique_ptr<TTNeighborsServiceChat> mServiceChat;
    std::unique_ptr<TTNeighborsServiceDiscovery> mServiceDiscovery;
    std::unique_ptr<TTServer> mServer;
    std::unique_ptr<TTNeighborsStub> mNeighborsStub;
    std::unique_ptr<TTBroadcasterChat> mBroadcasterChat;
    std::unique_ptr<TTBroadcasterDiscovery> mBroadcasterDiscovery;
};
