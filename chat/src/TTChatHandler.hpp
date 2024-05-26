#pragma once
#include "TTChatEntry.hpp"
#include "TTChatSettings.hpp"
#include <mqueue.h>
#include <memory>
#include <future>
#include <queue>
#include <vector>
#include <list>

class TTChatHandler {
public:
    explicit TTChatHandler(const TTChatSettings& settings);
    virtual ~TTChatHandler();
    TTChatHandler(const TTChatHandler&) = delete;
    TTChatHandler(const TTChatHandler&&) = delete;
    TTChatHandler operator=(const TTChatHandler&) = delete;
    TTChatHandler operator=(const TTChatHandler&&) = delete;
    virtual bool send(size_t id, std::string message, TTChatTimestamp timestamp);
    virtual bool receive(size_t id, std::string message, TTChatTimestamp timestamp);
    virtual bool clear(size_t id);
    virtual bool create(size_t id);
    virtual const TTChatEntries& get(size_t id);
private:
    bool send(TTChatMessageType type, std::string data, TTChatTimestamp timestamp);
    std::list<std::unique_ptr<TTChatMessage>> dequeue();
    // Receives heartbeat
    void heartbeat();
    // Sends heartbeat periodically or main data if available
    void main();
    // IPC message queue communication
    std::shared_ptr<TTUtilsMessageQueue> mPrimaryMessageQueue;
    std::shared_ptr<TTUtilsMessageQueue> mSecondaryMessageQueue;
    // Thread concurrent message communication
    std::atomic<bool> mForcedQuit;
    std::future<void> mHeartbeatResult;
    std::future<void> mHandlerResult;
    std::thread mHeartbeatThread;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::queue<std::unique_ptr<TTChatMessage>> mQueuedMessages;
    // Messages storage
    size_t mCurrentId;
    std::vector<TTChatEntries> mMessages;
};