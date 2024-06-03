#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTNeighborsChat.hpp"
#include "TTNeighborsDiscovery.hpp"

class TTNeighbors : public TTNeighborsChat, public TTNeighborsDiscovery {
public:
    TTNeighbors(TTNetworkInterface interface, std::deque<std::string> neighbors, TTContactsHandler& contactsHandler, TTChatHandler& chatHandler);
    virtual ~TTNeighbors();
    TTNeighbors(const TTNeighbors&) = delete;
    TTNeighbors(TTNeighbors&&) = delete;
    TTNeighbors& operator=(const TTNeighbors&) = delete;
    TTNeighbors& operator=(TTNeighbors&&) = delete;
    virtual bool handleTell(const TTNarrateMessage& message);
    virtual bool handleNarrate(const TTNarrateMessages& messages);
    virtual bool handleGreet(const TTGreetMessage& message);
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message);
    virtual std::string getNickname();
    virtual std::string getIdentity();
    virtual std::string getIpAddressAndPort();
private:
    TTNetworkInterface mInterface;
    std::deque<std::string> mNeighbors;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    std::deque<std::vector<TTNarrateMessage>> mUnorderedMessages;
    std::deque<std::vector<TTNarrateMessage>> mPendingMessages;
};
