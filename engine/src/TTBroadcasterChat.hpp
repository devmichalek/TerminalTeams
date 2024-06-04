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
    virtual void run();
    virtual void stop();
    virtual bool stopped() const;
    virtual bool handleTell(const TTNarrateMessage& message);
    virtual bool handleNarrate(const TTNarrateMessages& messages);
    virtual std::string getIdentity();
private:
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    std::deque<std::vector<TTNarrateMessage>> mUnorderedMessages;
    std::deque<std::vector<TTNarrateMessage>> mPendingMessages;
};
