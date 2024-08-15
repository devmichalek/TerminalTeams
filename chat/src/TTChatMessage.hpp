#pragma once
#include <string_view>
#include <chrono>
#include <cstring>
#include <cassert>
#include "TTChatMessageType.hpp"
#include "TTChatTimestamp.hpp"

class TTChatMessage {
public:
    TTChatMessage() {
        mDataLength = 0;
    }
    TTChatMessage(TTChatMessageType type, TTChatTimestamp timestamp = {}, const std::string_view& data = {}) {
        setType(type);
        setTimestamp(timestamp);
        setData(data);
    }
    ~TTChatMessage() = default;
    TTChatMessage(const TTChatMessage&) = default;
    TTChatMessage(TTChatMessage&&) = default;
    TTChatMessage& operator=(const TTChatMessage&) = default;
    TTChatMessage& operator=(TTChatMessage&&) = default;
    void setType(TTChatMessageType type) { mType = type; }
    void setTimestamp(TTChatTimestamp timestamp) { mTimestamp = timestamp; }
    void setData(const std::string_view& data) {
        assert(data.size() <= MAX_DATA_LENGTH);
        mDataLength = data.size();
        memcpy(mData, data.data(), data.size());
    }
    TTChatMessageType getType() const { return mType; }
    TTChatTimestamp getTimestamp() const { return mTimestamp; }
    std::string getData() const { return std::string(mData, mDataLength); }
    static constexpr unsigned int MAX_DATA_LENGTH = 2048;
private:
    TTChatMessageType mType;
    TTChatTimestamp mTimestamp;
    unsigned int mDataLength;
    char mData[MAX_DATA_LENGTH];
};

inline std::ostream& operator<<(std::ostream& os, const TTChatMessage& rhs)
{
    os << "{";
    os << "type: " << rhs.getType() << ", ";
    os << "timestamp: " << rhs.getTimestamp() << ", ";
    os << "data: " << rhs.getData();
    os << "}";
    return os;
}

inline bool operator==(const TTChatMessage& lhs, const TTChatMessage& rhs) {
    switch (lhs.getType()) {
        case TTChatMessageType::SENDER:
        case TTChatMessageType::SENDER_CHUNK:
        case TTChatMessageType::RECEIVER:
        case TTChatMessageType::RECEIVER_CHUNK:
            if (lhs.getType() != rhs.getType()) {
                return false;
            }
            break;
        case TTChatMessageType::CLEAR:
        case TTChatMessageType::HEARTBEAT:
        case TTChatMessageType::GOODBYE:
            return lhs.getType() == rhs.getType();
        default:
            return false;
    }
    if (lhs.getTimestamp() != rhs.getTimestamp()) {
        return false;
    }
    if (lhs.getData() != rhs.getData()) {
        return false;
    }
    return true;
}
