#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTBroadcasterStub.hpp"

class TTBroadcasterChat {
public:
    TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler, TTNetworkInterface interface);
    virtual ~TTBroadcasterChat();
    TTBroadcasterChat(const TTBroadcasterChat&) = delete;
    TTBroadcasterChat(TTBroadcasterChat&&) = delete;
    TTBroadcasterChat& operator=(const TTBroadcasterChat&) = delete;
    TTBroadcasterChat& operator=(TTBroadcasterChat&&) = delete;
    // Main loop
    virtual void run();
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Handles message (send)
    virtual bool handleSend(const std::string& message);
    // Handles request (receive)
    virtual bool handleReceive(const TTTellRequest& request);
    // Handles request (receive)
    virtual bool handleReceive(const TTNarrateRequest& request);
    // Returns root nickname
    virtual std::string getIdentity();
private:
    struct Neighbor {
        UniqueChatStub stub;
        std::deque<std::string> pendingMessages;
    };
    
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    TTNetworkInterface mInterface;
    TTBroadcasterStub mBroadcasterStub;
    std::mutex mNeighborsMutex;
    std::condition_variable mNeighborsCondition;
    std::map<size_t, Neighbor> mNeighbors;
    static inline const std::chrono::milliseconds mNeighborsTimeout{100};
};
