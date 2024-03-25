#pragma once
#include "TTChatEntry.hpp"
#include "TTChatCallback.hpp"
#include <mqueue.h>
#include <memory>
#include <future>
#include <queue>
#include <vector>

class TTChatHandler {
public:
    explicit TTChatHandler(std::string messageQueueName,
        TTChatCallbackMessageSent callbackMessageSent = {},
        TTChatCallbackMessageReceived callbackMessageReceived = {});
    ~TTChatHandler();
    bool send(size_t id, std::string message, TTChatTimestamp timestamp);
    bool receive(size_t id, std::string message, TTChatTimestamp timestamp);
    bool clear(size_t id);
    bool create(size_t id);
    const TTChatEntries& get(size_t id);

private:
    bool send(TTChatMessageType type, std::string data, TTChatTimestamp timestamp);
    void heartbeat();
    void main();
    // Callbacks
    TTChatCallbackMessageSent mCallbackMessageSent;
    TTChatCallbackMessageReceived mCallbackMessageReceived;
    // IPC main message queue communication
    std::string mMessageQueueName;
    mqd_t mMessageQueueDescriptor;
    // IPC reversed message queue communication
    std::string mMessageQueueReversedName;
    mqd_t mMessageQueueReversedDescriptor;
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