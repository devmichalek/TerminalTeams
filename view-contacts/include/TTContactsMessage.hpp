#pragma once
#include <semaphore.h>

enum TTContactsCommand : unsigned int {
    ACTIVE_CONTACT = 0,
    INACTIVE_CONTACT,
    NEW_CONTACT,
    NEW_MESSAGE
};

enum TTContactsSignal : int {
    SET,
    UNSET
};

const inline unsigned int TTCONTACTS_MAX_DATA_LENGTH = 0xFF;

// Directional message
struct TTContactsMessage {
    TTContactsCommand command;
    unsigned int dataLength;
    char data[TTCONTACTS_MAX_DATA_LENGTH];
};

struct TTContactsSharedMessage {
    sem_t semaphore;
    TTContactsMessage message;
};