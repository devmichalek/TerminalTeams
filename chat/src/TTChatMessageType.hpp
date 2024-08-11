#pragma once
#include <iostream>

enum class TTChatMessageType : unsigned int {
    CLEAR = 0,
    SENDER,
    RECEIVER,
    HEARTBEAT,
    GOODBYE
};

inline std::ostream& operator<<(std::ostream& os, const TTChatMessageType& rhs)
{
    switch (rhs) {
        case TTChatMessageType::CLEAR: os << "CLEAR"; break;
        case TTChatMessageType::SENDER: os << "SENDER"; break;
        case TTChatMessageType::RECEIVER: os << "RECEIVER"; break;
        case TTChatMessageType::HEARTBEAT: os << "HEARTBEAT"; break;
        case TTChatMessageType::GOODBYE: os << "GOODBYE"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}
