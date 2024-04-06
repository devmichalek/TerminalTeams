#pragma once
#include <functional>

using TTTextBoxCallbackMessageSent = std::function<void(std::string)>;
using TTTextBoxCallbackContactSwitch = std::function<void(size_t)>;

class TTTextBoxHandler {
public:
    explicit TTTextBoxHandler(std::string uniqueName,
        TTTextBoxCallbackMessageSent callbackMessageSent,
        TTTextBoxCallbackContactSwitch callbackContactsSwitch);
    ~TTTextBoxHandler();
private:
    void heartbeat(std::promise<void> promise);
    void main(std::promise<void> promise);
    // Callbacks
    TTTextBoxCallbackMessageSent mCallbackMessageSent;
    TTTextBoxCallbackContactSwitch mCallbackContactsSwitch;
    // IPC communication
    int mNamedPipeDescriptor;
    int mSocketDescriptor;
    std::string mNamedPipePath;
    // Thread concurrent message communication
    std::atomic<bool> mStopped;
    std::queue<std::thread> mThreads;
    std::queue<std::future<void>> mBlockers;
};