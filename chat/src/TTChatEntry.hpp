#pragma once
#include "TTChatMessageType.hpp"
#include "TTChatTimestamp.hpp"
#include <string>
#include <deque>

struct TTChatEntry final {
    TTChatEntry(TTChatMessageType type, TTChatTimestamp timestamp, const std::string& data) :
        type(type), timestamp(timestamp), data(data) {}
    ~TTChatEntry() = default;
    TTChatEntry(const TTChatEntry&) = default;
    TTChatEntry(TTChatEntry&&) = default;
    TTChatEntry& operator=(const TTChatEntry&) = default;
    TTChatEntry& operator=(TTChatEntry&&) = default;
    TTChatMessageType type;
    TTChatTimestamp timestamp;
    std::string data;
};

using TTChatEntries = std::deque<TTChatEntry>;

inline std::ostream& operator<<(std::ostream& os, const TTChatEntry& rhs)
{
    os << "{";
    os << "type: " << rhs.type << ", ";
    os << "timestamp: " << rhs.timestamp << ", ";
    os << "data: " << rhs.data;
    os << "}";
    return os;
}

inline bool operator==(const TTChatEntry& lhs, const TTChatEntry& rhs) {
    switch (lhs.type) {
        case TTChatMessageType::SENDER:
        case TTChatMessageType::RECEIVER:
            if (lhs.type != rhs.type) {
                return false;
            }
            break;
        default:
            return false;
    }
    if (lhs.timestamp != rhs.timestamp) {
        return false;
    }
    if (lhs.data != rhs.data) {
        return false;
    }
    return true;
}
