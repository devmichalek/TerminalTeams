#pragma once

enum class TTChatMessageType : unsigned int {
    CLEAR = 0,
    SENDER,
    RECEIVER,
    HEARTBEAT,
    GOODBYE
};
