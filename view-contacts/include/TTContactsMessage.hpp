#pragma once
#include "TTContactsStatus.hpp"

inline const unsigned int TTCONTACTS_DATA_MAX_LENGTH = 256;
inline const char* const TTCONTACTS_DATA_PRODUCED_POSTFIX = "-data-produced";
inline const char* const TTCONTACTS_DATA_CONSUMED_POSTFIX = "-data-consumed";
inline const long TTCONTACTS_SEMAPHORES_READY_TRY_COUNT = 5; // 5 times
inline const long TTCONTACTS_SEMAPHORES_READY_TIMEOUT_MS = 2000; // 1Ms
inline const long TTCONTACTS_DATA_PRODUCE_TRY_COUNT = 2; // 2 times
inline const long TTCONTACTS_DATA_PRODUCE_TIMEOUT_S = 1; // 1s
inline const long TTCONTACTS_DATA_CONSUME_TRY_COUNT = 3; // 3 times
inline const long TTCONTACTS_DATA_CONSUME_TIMEOUT_S = 1; // 1s

// Directional message
struct TTContactsMessage {
    size_t id;
    TTContactsStatus status;
    unsigned int dataLength;
    char data[TTCONTACTS_DATA_MAX_LENGTH];
};
