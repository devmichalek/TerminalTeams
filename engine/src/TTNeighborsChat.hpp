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
};

using TTNarrateMessages = std::list<TTNarrateMessage>;

class TTNeighborsChat {
public:
    TTNeighborsChat() = default;
    virtual ~TTNeighborsChat() = default;
    TTNeighborsChat(const TTNeighborsChat&) = default;
    TTNeighborsChat(TTNeighborsChat&&) = default;
    TTNeighborsChat& operator=(const TTNeighborsChat&) = default;
    TTNeighborsChat& operator=(TTNeighborsChat&&) = default;
    virtual bool handleTell(const TTNarrateMessage& message) = 0;
    virtual bool handleNarrate(const TTNarrateMessages& messages) = 0;
    virtual std::string getIdentity() const = 0;
};

