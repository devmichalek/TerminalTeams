#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsMessage.hpp"
#include <semaphore.h>

class TTContacts {
public:
    TTContacts(TTContactsSettings settings);
    ~TTContacts();
    void run();
private:
    std::string mSharedName;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
};