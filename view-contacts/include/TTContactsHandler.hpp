#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsMessage.hpp"
#include <semaphore.h>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>

class TTContactsHandler {
public:
    TTContactsHandler(std::string sharedName);
    ~TTContactsHandler();
    void send(const TTContactsMessage& message);
private:
    void run();
    std::string mSharedName;
    std::queue<std::unique_ptr<TTContactsMessage>> mQueuedMessages;
    mutable std::mutex mQueueMutex;
    mutable std::condition_variable mQueueCondition;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
};