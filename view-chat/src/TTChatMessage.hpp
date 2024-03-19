#pragma once
#include <chrono>

enum TTChatMessageType : unsigned int {
    CLEAR = 0,
    SEND,
    RECEIVE,
    HEARTBEAT
};

using TTChatTimestamp = std::chrono::time_point<std::chrono::system_clock>;

inline const unsigned int TTCHAT_DATA_MAX_LENGTH = 1024;

struct TTChatMessage {
    TTChatMessage(TTChatMessageType type,
        TTChatTimestamp timestamp,
        unsigned int dataLength,
        const char* data) :
        type(type),
        timestamp(timestamp) {
        std::memcpy(this->data, data, dataLength);
    }
    TTChatMessageType type;
    TTChatTimestamp timestamp;
    char data[TTCHAT_DATA_MAX_LENGTH];
};

inline const unsigned int TTCHAT_MESSAGE_MAX_LENGTH = sizeof(TTChatMessage);
inline const unsigned int TTCHAT_MESSAGE_MAX_NUM = 16;
inline const unsigned int TTCHAT_MESSAGE_PRIORITY = 0;
inline const long TTCHAT_MESSAGE_SEND_TIMEOUT_S = 1; // 1s
inline const long TTCHAT_MESSAGE_RECEIVE_TIMEOUT_S = TTCHAT_MESSAGE_SEND_TIMEOUT_S; // 1s
inline const int TTCHAT_MESSAGE_SEND_TRY_COUNT = 3; // 3 times
inline const int TTCHAT_MESSAGE_RECEIVE_TRY_COUNT = TTCHAT_MESSAGE_SEND_TRY_COUNT; // 3 times
