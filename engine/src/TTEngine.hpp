#pragma once
#include "TTEngineSettings.hpp"
#include "TTUtilsStopper.hpp"

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
    [[nodiscard]] virtual bool stopped() const;
private:
    // Server thread
    void server(std::promise<void> promise);
    // Broadcaster chat thread
    void chat(std::promise<void> promise);
    // Broadcaster discovery thread
    void discovery(std::promise<void> promise);
    // Callback mailbox function
    void mailbox(const std::string& message);
    // Callback switcher function (contacts selection)
    void switcher(size_t message);
    // Stops application (internal function)
    virtual void stopInternal();
    // Concurrent communication
    TTUtilsStopper mStopper;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
    // Handlers, IPC communication
    std::unique_ptr<TTContactsHandler> mContacts;
    std::unique_ptr<TTChatHandler> mChat;
    std::unique_ptr<TTTextBoxHandler> mTextBox;
    std::mutex mExternalCallsMutex;
    // Node data
    std::unique_ptr<TTServer> mServer;
    std::unique_ptr<TTNeighborsStub> mNeighborsStub;
    std::unique_ptr<TTBroadcasterChat> mBroadcasterChat;
    std::unique_ptr<TTBroadcasterDiscovery> mBroadcasterDiscovery;
};
