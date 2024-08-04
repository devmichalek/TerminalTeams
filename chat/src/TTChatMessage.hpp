#pragma once
#include <string>
#include <string_view>
#include <chrono>
#include <cstring>
#include <cassert>
#include "TTChatMessageType.hpp"

using TTChatTimestamp = std::chrono::time_point<std::chrono::system_clock>;

class TTChatMessage {
public:
    TTChatMessage() = default;
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

inline const unsigned int TTCHAT_MESSAGE_MAX_NUM = 8;
inline const unsigned int TTCHAT_MESSAGE_PRIORITY = 0;
inline const long TTCHAT_MESSAGE_SEND_TIMEOUT_S = 1; // 1s
inline const long TTCHAT_MESSAGE_RECEIVE_TIMEOUT_S = TTCHAT_MESSAGE_SEND_TIMEOUT_S; // 1s
inline const int TTCHAT_MESSAGE_SEND_TRY_COUNT = 3; // 3 times
inline const int TTCHAT_MESSAGE_RECEIVE_TRY_COUNT = TTCHAT_MESSAGE_SEND_TRY_COUNT; // 3 times
inline const long TTCHAT_HEARTBEAT_TIMEOUT_MS = 500; // 0.5s
inline const long TTCHAT_MESSAGE_QUEUE_READY_TRY_COUNT = 5; // 5 times
inline const long TTCHAT_MESSAGE_QUEUE_READY_TIMEOUT_MS = 1000; // 1s