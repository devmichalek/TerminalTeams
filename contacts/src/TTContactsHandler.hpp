#pragma once
#include "TTContactsMessage.hpp"
#include "TTContactsEntry.hpp"
#include "TTContactsSettings.hpp"
#include "TTUtilsSharedMem.hpp"
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
    explicit TTContactsHandler(const TTContactsSettings& settings);
    virtual ~TTContactsHandler();
    TTContactsHandler(const TTContactsHandler&) = delete;
    TTContactsHandler(TTContactsHandler&&) = delete;
    TTContactsHandler& operator=(const TTContactsHandler&) = delete;
    TTContactsHandler& operator=(TTContactsHandler&&) = delete;
    virtual bool create(std::string nickname, std::string fullname, std::string ipAddressAndPort);
    virtual bool send(size_t id);
    virtual bool receive(size_t id);
    virtual bool activate(size_t id);
    virtual bool deactivate(size_t id);
    virtual bool select(size_t id);
    virtual bool unselect(size_t id);
    virtual const TTContactsEntry& get(size_t id) const;
private:
    // Send generic method
    bool send(const TTContactsMessage& message);
    // Sends heartbeat periodically
    void heartbeat();
    // Sends main data if available and receives confirmation
    void main();
    // Establish connection with the other process
    bool establish();
    // IPC shared memory communication
    std::shared_ptr<TTUtilsSharedMem> mSharedMem;
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