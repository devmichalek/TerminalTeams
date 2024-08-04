#pragma once

enum class TTChatMessageType : unsigned int {
    CLEAR = 0,
    SEND,
    RECEIVE,
    HEARTBEAT,
    GOODBYE
};
