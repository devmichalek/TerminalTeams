#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTNeighborsChat.hpp"
#include "TTNeighborsDiscovery.hpp"

class TTBroadcasterChat : public TTNeighborsChat {
public:
    TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler);
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
    // Tell message handler
    virtual bool handleTell(const TTNarrateMessage& message);
    // Narrate message handler
    virtual bool handleNarrate(const TTNarrateMessages& messages);
    // Returns root nickname
    virtual const std::string& getIdentity() const;
private:
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    std::deque<std::vector<TTNarrateMessage>> mUnorderedMessages;
    std::deque<std::vector<TTNarrateMessage>> mPendingMessages;
};
