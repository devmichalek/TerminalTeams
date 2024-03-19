#pragma once
#include "TTChatMessage.hpp"
#include <mqueue.h>
#include <memory>
#include <future>
#include <queue>

class TTChatHandler {
public:
    explicit TTChatHandler(std::string messageQueueName);
    ~TTChatHandler();
    bool send(std::string message, TTChatTimestamp timestamp);
    bool receive(std::string message, TTChatTimestamp timestamp);
    bool clear();

private:
    bool divide(TTChatMessageType type, std::string message, TTChatTimestamp timestamp);
    bool send(std::unique_ptr<TTChatMessage> message);
    void heartbeat();
    void main();
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
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::queue<std::unique_ptr<TTChatMessage>> mQueuedMessages;
};