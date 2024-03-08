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

// Class ment to be embedded into other higher abstract class.
// Allows to control TTContacts process concurrently.
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
    void main();
    // IPC shared memory communication
    std::string mSharedName;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    static inline const long DATA_CONSUME_TRY_COUNT = 3; // 3 times
    static inline const long DATA_CONSUME_TIMEOUT_NS = 500000000; // 0.5s
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTContactsMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::atomic<bool> mThreadForceExit;
    std::thread mHandlerThread;
    // Contacts storage
    std::vector<TTContactsEntry> mContacts;
};