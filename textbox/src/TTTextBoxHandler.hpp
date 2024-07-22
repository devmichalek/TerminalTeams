#pragma once
#include <string>
#include <future>
#include <deque>
#include <functional>
#include "TTUtilsNamedPipe.hpp"
#include "TTTextBoxSettings.hpp"

using TTTextBoxCallbackMessageSent = std::function<void(const std::string&)>;
using TTTextBoxCallbackContactSwitch = std::function<void(size_t)>;

// Class meant to be embedded into other higher abstract class.
// Allows to control TTTextBox process concurrently.
class TTTextBoxHandler {
public:
    explicit TTTextBoxHandler(const TTTextBoxSettings& settings,
        TTTextBoxCallbackMessageSent callbackMessageSent,
        TTTextBoxCallbackContactSwitch callbackContactsSwitch);
    virtual ~TTTextBoxHandler();
    TTTextBoxHandler(const TTTextBoxHandler&) = delete;
    TTTextBoxHandler(TTTextBoxHandler&&) = delete;
    TTTextBoxHandler& operator=(const TTTextBoxHandler&) = delete;
    TTTextBoxHandler& operator=(TTTextBoxHandler&&) = delete;
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
};