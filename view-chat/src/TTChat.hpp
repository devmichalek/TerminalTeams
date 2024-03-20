#pragma once
#include "TTChatMessage.hpp"
#include "TTChatCallback.hpp"

class TTChat {
public:
    explicit TTChat(TTChatSettings settings,
        TTChatCallbackQuit callbackQuit = {});
    void run();
private:
    void heartbeat();
    void print(const char* cmessage, TTChatTimestamp timestmap, bool received);
    void clear();
    // Callbacks
    TTChatCallbackQuit mCallbackQuit;
    // IPC message queue communication
    mqd_t mMessageQueueDescriptor;
    mqd_t mMessageQueueReversedDescriptor;
    // Support data
    size_t mWidth;
    size_t mHeight;
    size_t mSideWidth;
    std::string mBlankLine;
};

