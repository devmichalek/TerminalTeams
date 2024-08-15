#pragma once
#include <iostream>

enum class TTChatMessageType : unsigned int {
    CLEAR = 0,
    SENDER,
    SENDER_CHUNK,
    RECEIVER,
    RECEIVER_CHUNK,
    HEARTBEAT,
    GOODBYE
};

inline std::ostream& operator<<(std::ostream& os, const TTChatMessageType& rhs)
{
    switch (rhs) {
        case TTChatMessageType::CLEAR: os << "CLEAR"; break;
        case TTChatMessageType::SENDER: os << "SENDER"; break;
        case TTChatMessageType::SENDER_CHUNK: os << "SENDER_CHUNK"; break;
        case TTChatMessageType::RECEIVER: os << "RECEIVER"; break;
        case TTChatMessageType::RECEIVER_CHUNK: os << "RECEIVER_CHUNK"; break;
        case TTChatMessageType::HEARTBEAT: os << "HEARTBEAT"; break;
        case TTChatMessageType::GOODBYE: os << "GOODBYE"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}
