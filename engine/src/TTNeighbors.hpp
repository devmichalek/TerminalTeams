#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTInterface.hpp"
#include "TTNeighborsChat.hpp"
#include "TTNeighborsDiscovery.hpp"

class TTNeighbors : public TTNeighborsChat, TTNeighborsDiscovery {
public:
    TTNeighbors(TTInterface interface, TTContactsHandler& contactsHandler, TTChatHandler& chatHandler);
    virtual ~TTNeighbors();
    TTNeighbors(const TTNeighbors&) = delete;
    TTNeighbors(TTNeighbors&&) = delete;
    TTNeighbors operator=(const TTNeighbors&) = delete;
    TTNeighbors operator=(TTNeighbors&&) = delete;
    virtual bool handleTell(const TTNarrateMessage& message);
    virtual bool handleNarrate(const TTNarrateMessages& messages);
    virtual bool handleGreet(const TTGreetMessage& message);
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message);
    virtual std::string getNickname();
    virtual std::string getIdentity();
    virtual std::string getIpAddressAndPort();
private:
    TTInterface mInterface;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;

    std::deque<std::vector<TTNarrateMessage>> mUnorderedMessages;
    std::deque<std::vector<TTNarrateMessage>> mPendingMessages;
    std::deque<std::string> mNeighbors;
};
