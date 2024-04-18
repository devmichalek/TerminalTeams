#pragma once
#include "TTContactsStatus.hpp"

inline const unsigned int TTCONTACTS_DATA_MAX_LENGTH = 256;
inline const char* const TTCONTACTS_DATA_PRODUCED_POSTFIX = "-data-produced";
inline const char* const TTCONTACTS_DATA_CONSUMED_POSTFIX = "-data-consumed";
inline const long TTCONTACTS_DATA_CONSUME_TRY_COUNT = 3; // 3 times
inline const long TTCONTACTS_DATA_CONSUME_TIMEOUT_S = 1; // 1s
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
    constexpr TTContactsMessage& operator=(const TTContactsMessage&) = default;
    constexpr TTContactsMessage& operator=(TTContactsMessage&&) = default;
};
