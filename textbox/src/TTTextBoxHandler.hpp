#pragma once
#include <string>
#include <future>
#include <deque>
#include <functional>
#include "TTUtilsNamedPipe.hpp"

using TTTextBoxCallbackMessageSent = std::function<void(std::string)>;
using TTTextBoxCallbackContactSwitch = std::function<void(size_t)>;

class TTTextBoxHandler {
public:
    explicit TTTextBoxHandler(const TTTextBoxSettings& settings,
        TTTextBoxCallbackMessageSent callbackMessageSent,
        TTTextBoxCallbackContactSwitch callbackContactsSwitch);
    virtual ~TTTextBoxHandler();
    TTTextBoxHandler(const TTTextBoxHandler&) = delete;
    TTTextBoxHandler(const TTTextBoxHandler&&) = delete;
    TTTextBoxHandler operator=(const TTTextBoxHandler&) = delete;
    TTTextBoxHandler operator=(const TTTextBoxHandler&&) = delete;
    virtual void stop();
    virtual bool stopped() const;
private:
    void main(std::promise<void> promise);
    // IPC communication
    std::shared_ptr<TTUtilsNamedPipe> mPipe;
    // Callbacks
    TTTextBoxCallbackMessageSent mCallbackMessageSent;
    TTTextBoxCallbackContactSwitch mCallbackContactsSwitch;
    // Thread concurrent message communication
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
    // Logger
    inline static const std::string mClassNamePrefix = "TTTextBox:";
};