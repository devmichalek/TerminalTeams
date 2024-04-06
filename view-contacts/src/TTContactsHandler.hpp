#pragma once
#include "TTContactsMessage.hpp"
#include "TTContactsEntry.hpp"
#include "TTContactsCallback.hpp"
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
    explicit TTContactsHandler(std::string sharedMemoryName,
        TTContactsCallbackDataProduced callbackDataProduced = {},
        TTContactsCallbackDataConsumed callbackDataConsumed = {});
    ~TTContactsHandler();
    bool create(std::string nickname, std::string fullname, std::string decription, std::string ipAddressAndPort);
    bool send(size_t id);
    bool receive(size_t id);
    bool activate(size_t id);
    bool deactivate(size_t id);
    bool select(size_t id);
    bool unselect(size_t id);
    const TTContactsEntry& get(size_t id) const;
private:
    bool send(const TTContactsMessage& message);
    // Sends heartbeat periodically
    void heartbeat();
    // Sends main data if available and receives confirmation
    void main();
    // Callbacks
    TTContactsCallbackDataProduced mCallbackDataProduced;
    TTContactsCallbackDataConsumed mCallbackDataConsumed;
    // IPC shared memory communication
    std::string mSharedMemoryName;
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Quit flag
    std::atomic<bool> mForcedQuit;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTContactsMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::thread mHandlerThread;
    std::mutex mHandlerQuitMutex;
    // Thread concurrent Heartbeat
    std::thread mHeartbeatThread;
    std::mutex mHeartbeatQuitMutex;
    // Contacts storage
    std::vector<TTContactsEntry> mContacts;
};