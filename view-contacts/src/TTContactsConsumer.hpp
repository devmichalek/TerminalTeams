#pragma once
#include "TTContactsMessage.hpp"
#include "TTContactsSyscall.hpp"
#include <memory>

class TTContactsConsumer {
public:
    explicit TTContactsConsumer(const std::string& sharedMemoryName,
        const std::string& dataConsumedSemName,
        const std::string& dataProducedSemName,
        const TTContactsSyscall& syscall);
    virtual ~TTContactsConsumer() {}
    TTContactsConsumer(const TTContactsConsumer&) = delete;
    TTContactsConsumer(TTContactsConsumer&&) = delete;
    constexpr TTContactsConsumer& operator=(const TTContactsConsumer&) = delete;
    constexpr TTContactsConsumer& operator=(TTContactsConsumer&&) = delete;
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
    const TTContactsSyscall& mSyscall;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Flags
    bool mAlive;
};