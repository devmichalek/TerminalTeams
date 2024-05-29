#pragma once

class TTNeighborsChat {
public:
    TTNeighborsChat() = default;
    virtual ~TTNeighborsChat();
    TTNeighborsChat(const TTNeighborsChat&) = delete;
    TTNeighborsChat(TTNeighborsChat&&) = delete;
    TTNeighborsChat operator=(const TTNeighborsChat&) = delete;
    TTNeighborsChat operator=(TTNeighborsChat&&) = delete;
private:
};
