#pragma once
#include "TTChatSettings.hpp"
#include "TTChatMessage.hpp"
#include "TTChatCallback.hpp"
#include "TTChatMessageQueue.hpp"
#include <future>

class TTChat {
public:
    explicit TTChat(TTChatSettings settings, TTChatCallbackQuit callbackQuit = {});
    ~TTChat();
    // Receives main data
    void run();
private:
    // Sends heartbeat periodically
    void heartbeat(std::promise<void> promise);
    // Prints message in a defined format
    void print(const char* cmessage, TTChatTimestamp timestmap, bool received);
    // Callbacks
    TTChatCallbackQuit mCallbackQuit;
    // IPC message queue communication
    mqd_t mMessageQueueDescriptor;
    mqd_t mMessageQueueReversedDescriptor;
    // Thread concurrent message communication
    std::atomic<bool> mForcedQuit;
    std::future<void> mHeartbeatResult;
    std::thread mHeartbeatThread;
    // Real data
    size_t mWidth;
    size_t mHeight;
    size_t mSideWidth;
    std::string mBlankLine;
};
