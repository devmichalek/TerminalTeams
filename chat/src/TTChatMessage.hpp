#pragma once
#include <chrono>
#include <cstring>

enum class TTChatMessageType : unsigned int {
    CLEAR = 0,
    SEND,
    RECEIVE,
    HEARTBEAT
};

using TTChatTimestamp = std::chrono::time_point<std::chrono::system_clock>;

inline const unsigned int TTCHAT_DATA_MAX_LENGTH = 2048;

struct TTChatMessage {
    TTChatMessage() = default;
    TTChatMessage(TTChatMessageType type,
        TTChatTimestamp timestamp,
        unsigned int dataLength,
        const char* data) :
            type(type),
            timestamp(timestamp),
            dataLength(dataLength) {
        memcpy(this->data, data, dataLength);
    }
    TTChatMessageType type;
    TTChatTimestamp timestamp;
    unsigned int dataLength;
    char data[TTCHAT_DATA_MAX_LENGTH];
};

inline const unsigned int TTCHAT_MESSAGE_MAX_LENGTH = sizeof(TTChatMessage);
inline const unsigned int TTCHAT_MESSAGE_MAX_NUM = 8;
inline const unsigned int TTCHAT_MESSAGE_PRIORITY = 0;
inline const long TTCHAT_MESSAGE_SEND_TIMEOUT_S = 1; // 1s
inline const long TTCHAT_MESSAGE_RECEIVE_TIMEOUT_S = TTCHAT_MESSAGE_SEND_TIMEOUT_S; // 1s
inline const int TTCHAT_MESSAGE_SEND_TRY_COUNT = 3; // 3 times
inline const int TTCHAT_MESSAGE_RECEIVE_TRY_COUNT = TTCHAT_MESSAGE_SEND_TRY_COUNT; // 3 times
inline const long TTCHAT_HEARTBEAT_TIMEOUT_MS = 500; // 0.5s
inline const long TTCHAT_MESSAGE_QUEUE_READY_TRY_COUNT = 5; // 5 times
inline const long TTCHAT_MESSAGE_QUEUE_READY_TIMEOUT_MS = 1000; // 1s