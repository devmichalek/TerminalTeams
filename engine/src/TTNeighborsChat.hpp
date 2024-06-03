#pragma once
#include <string>
#include <chrono>
#include <list>

struct TTNarrateMessage {
    std::string identity;
    std::string message;
    int sequenceNumber;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    bool senderSide;
}

using TTNarrateMessages = std::list<TTNarrateMessage>;

class TTNeighborsChat {
public:
    virtual ~TTNeighborsChat();
    virtual bool handleTell(const TTNarrateMessage& message) = 0;
    virtual bool handleNarrate(const TTNarrateMessages& messages) = 0;
    virtual std::string getIdentity() = 0;
protected:
    TTNeighborsChat() = default;
    TTNeighborsChat(const TTNeighborsChat&) = delete;
    TTNeighborsChat(TTNeighborsChat&&) = delete;
    TTNeighborsChat operator=(const TTNeighborsChat&) = delete;
    TTNeighborsChat operator=(TTNeighborsChat&&) = delete;
};

