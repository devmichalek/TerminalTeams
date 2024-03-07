#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsMessage.hpp"
#include <semaphore.h>
#include <vector>
#include <string>

class TTContacts {
public:
    TTContacts(TTContactsSettings settings);
    ~TTContacts();
    void run();
private:
    // IPC shared memory communication
    std::string mSharedName;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<std::tuple<size_t, std::string, TTContactsStatus>> mContacts;
};