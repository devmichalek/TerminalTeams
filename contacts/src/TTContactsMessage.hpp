#pragma once
#include "TTContactsStatus.hpp"

inline const unsigned int TTCONTACTS_DATA_MAX_LENGTH = 256;
inline const long TTCONTACTS_HEARTBEAT_TIMEOUT_MS = 500; // 0.5s

// Directional message
struct TTContactsMessage final {
    size_t id;
    TTContactsStatus status;
    unsigned int dataLength;
    char data[TTCONTACTS_DATA_MAX_LENGTH];
    TTContactsMessage() = default;
    ~TTContactsMessage() = default;
    TTContactsMessage(const TTContactsMessage&) = default;
    TTContactsMessage(TTContactsMessage&&) = default;
    TTContactsMessage& operator=(const TTContactsMessage&) = default;
    TTContactsMessage& operator=(TTContactsMessage&&) = default;
};
