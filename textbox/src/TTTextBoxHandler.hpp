#pragma once
#include "TTUtilsNamedPipe.hpp"
#include "TTTextBoxSettings.hpp"
#include "TTUtilsStopable.hpp"
#include <string>
#include <future>
#include <deque>
#include <functional>

using TTTextBoxCallbackMessageSent = std::function<void(const std::string&)>;
using TTTextBoxCallbackContactSwitch = std::function<void(size_t)>;

// Class meant to be embedded into other higher abstract class.
// Allows to control TTTextBox process concurrently.
class TTTextBoxHandler : public TTUtilsStopable {
public:
    explicit TTTextBoxHandler(const TTTextBoxSettings& settings,
        TTTextBoxCallbackMessageSent callbackMessageSent,
        TTTextBoxCallbackContactSwitch callbackContactsSwitch);
    virtual ~TTTextBoxHandler();
    TTTextBoxHandler(const TTTextBoxHandler&) = delete;
    TTTextBoxHandler(TTTextBoxHandler&&) = delete;
    TTTextBoxHandler& operator=(const TTTextBoxHandler&) = delete;
    TTTextBoxHandler& operator=(TTTextBoxHandler&&) = delete;
protected:
    TTTextBoxHandler() = default;
private:
    void main(std::promise<void> promise);
    // Literals
    inline const static long RECEIVE_TIMEOUT_MS = 500;
    inline const static long RECEIVE_TRY_COUNT = 3;
    // IPC communication
    std::shared_ptr<TTUtilsNamedPipe> mPipe;
    // Callbacks
    TTTextBoxCallbackMessageSent mCallbackMessageSent;
    TTTextBoxCallbackContactSwitch mCallbackContactsSwitch;
    // Thread concurrent message communication
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};