#pragma once
#include <vector>
#include <string>
#include <chrono>

enum class TTChatSide {
    LEFT,
    RIGHT
};

using TTChatTimestamp = std::chrono::time_point<std::chrono::system_clock>;

struct TTChatMessage {
    explicit TTChatMessage(TTChatSide side, TTChatTimestamp timestamp, std::string message) : 
        side(side), timestamp(timestamp), message(message) {}
    TTChatSide side;
    TTChatTimestamp timestamp;
    std::string message;
};

using TTChatMessages = std::vector<TTChatMessage>;