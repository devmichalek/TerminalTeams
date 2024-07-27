#pragma once
#include <string.h>
#include "TTTextBoxStatus.hpp"

struct TTTextBoxMessage {
    explicit TTTextBoxMessage(TTTextBoxStatus status, unsigned dataLength, const char* src) :
        status(status), dataLength(dataLength) {
        memset(data, 0, DATA_MAX_LENGTH);
        memcpy(data, src, dataLength);
    }
    ~TTTextBoxMessage() = default;
    TTTextBoxMessage(const TTTextBoxMessage&) = default;
    TTTextBoxMessage(TTTextBoxMessage&&) = default;
    TTTextBoxMessage& operator=(const TTTextBoxMessage&) = default;
    TTTextBoxMessage& operator=(TTTextBoxMessage&&) = default;
    inline const static unsigned int DATA_MAX_DIGITS = 4;
    inline const static unsigned int DATA_MAX_LENGTH = 2048;
    TTTextBoxStatus status;
    unsigned int dataLength;
    char data[DATA_MAX_LENGTH];
};

inline std::ostream& operator<<(std::ostream& os, const TTTextBoxMessage& rhs)
{
    os << "{";
    os << "status: " << rhs.status << ", ";
    os << "length: " << rhs.dataLength << ", ";
    os << "message: " << rhs.data;
    os << "}";
    return os;
}

inline bool operator==(const TTTextBoxMessage& lhs, const TTTextBoxMessage& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.dataLength != rhs.dataLength) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, TTTextBoxMessage::DATA_MAX_LENGTH) == 0;
}
