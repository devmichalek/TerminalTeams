#pragma once
#include <string.h>

inline const long TTTEXTBOX_SEND_TIMEOUT_MS = 500;
inline const long TTTEXTBOX_RECEIVE_TIMEOUT_MS = 500;
inline const long TTTEXTBOX_QUEUED_MSG_TIMEOUT_MS = 500;
inline const long TTTEXTBOX_NAMED_PIPE_READY_TIMEOUT_MS = 500;
inline const long TTTEXTBOX_RECEIVE_TRY_COUNT = 3;
inline const long TTTEXTBOX_NAMED_PIPE_READY_TRY_COUNT = 3;
inline const unsigned int TTTEXTBOX_DATA_MAX_DIGITS = 4;
inline const unsigned int TTTEXTBOX_DATA_MAX_LENGTH = 2048;

enum class TTTextBoxStatus : unsigned int {
    UNDEFINED = 0,
    HEARTBEAT,
    CONTACTS_SWITCH,
    MESSAGE
};

struct TTTextBoxMessage {
    explicit TTTextBoxMessage(TTTextBoxStatus status, unsigned dataLength, const char* src) :
        status(status), dataLength(dataLength) {
        memcpy(data, src, dataLength);
    }
    TTTextBoxStatus status;
    unsigned int dataLength;
    char data[TTTEXTBOX_DATA_MAX_LENGTH];
};
