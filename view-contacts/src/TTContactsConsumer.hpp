#pragma once
#include "TTContactsMessage.hpp"
#include <semaphore.h>
#include <memory>

class TTContactsConsumer {
public:
    explicit TTContactsConsumer(const std::string& sharedMemoryName,
        const std::string& dataConsumedSemName,
        const std::string& dataProducedSemName);
    // Initialize system objects
    virtual bool init(long attempts = 5, long timeoutMs = 1000);
    // Get with timeout
    virtual std::unique_ptr<TTContactsMessage> get(long attempts = 2, long timeoutMs = 2000);
    // Check if object is functioning properly
    virtual bool alive() const;
private:
    // System objects names
    std::string mSharedMemoryName;
    std::string mDataConsumedSemName;
    std::string mDataProducedSemName;
    // IPC shared memory communication
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Flags
    bool mAlive;
};