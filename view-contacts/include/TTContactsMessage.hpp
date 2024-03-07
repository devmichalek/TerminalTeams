#pragma once
#include "TTContactsStatus.hpp"

const inline unsigned int TTCONTACTS_DATA_MAX_LENGTH = 256;
const inline char* const TTCONTACTS_DATA_PRODUCED_POSTFIX = "-data-produced";
const inline char* const TTCONTACTS_DATA_CONSUMED_POSTFIX = "-data-consumed";

// Directional message
struct TTContactsMessage {
    size_t id;
    TTContactsStatus status;
    unsigned int dataLength;
    char data[TTCONTACTS_DATA_MAX_LENGTH];
};
