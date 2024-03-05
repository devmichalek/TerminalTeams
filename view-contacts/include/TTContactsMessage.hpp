#pragma once

enum TTContactsCommand : unsigned int {
    ACTIVE_CONTACT = 0,
    INACTIVE_CONTACT,
    NEW_CONTACT,
    NEW_MESSAGE
};

const char* const TTCONTACTS_DATA_PRODUCED_POSTFIX = "-data-produced";
const char* const TTCONTACTS_DATA_CONSUMED_POSTFIX = "-data-consumed";
const inline unsigned int TTCONTACTS_MAX_DATA_LENGTH = 0xFF;

// Directional message
struct TTContactsMessage {
    TTContactsCommand command;
    unsigned int dataLength;
    char data[TTCONTACTS_MAX_DATA_LENGTH];
    TTContactsMessage() = default;
};
