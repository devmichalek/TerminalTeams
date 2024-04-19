#pragma once
#include <string>
#include <future>
#include <deque>
#include <functional>

using TTTextBoxCallbackMessageSent = std::function<void(std::string)>;
using TTTextBoxCallbackContactSwitch = std::function<void(size_t)>;

class TTTextBoxHandler {
public:
    explicit TTTextBoxHandler(std::string uniqueName,
        TTTextBoxCallbackMessageSent callbackMessageSent,
        TTTextBoxCallbackContactSwitch callbackContactsSwitch);
    ~TTTextBoxHandler();
    void stop();
    bool stopped() const;
private:
    void main(std::promise<void> promise);
    // Callbacks
    TTTextBoxCallbackMessageSent mCallbackMessageSent;
    TTTextBoxCallbackContactSwitch mCallbackContactsSwitch;
    // IPC communication
    int mNamedPipeDescriptor;
    std::string mNamedPipePath;
    // Thread concurrent message communication
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};