#pragma once
#include "TTContactsMessage.hpp"
#include "TTContactsEntry.hpp"
#include <semaphore.h>
#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>

class TTContactsHandler {
public:
    TTContactsHandler(std::string sharedName);
    ~TTContactsHandler();
    void create(std::string nickname, std::string fullname, std::string decription, std::string ipAddressAndPort);
    bool send(size_t id);
    bool receive(size_t id);
    bool activate(size_t id);
    bool deactivate(size_t id);
    bool select(size_t id);
    const TTContactsEntry& get(size_t id);
private:
    void send(const TTContactsMessage& message);
    void clean();
    void run();
    // IPC shared memory communication
    std::string mSharedName;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTContactsMessage>> mQueuedMessages;
    mutable std::mutex mQueueMutex;
    mutable std::condition_variable mQueueCondition;
    // Contacts storage
    std::vector<TTContactsEntry> mContacts;
};