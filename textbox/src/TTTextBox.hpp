#pragma once
#include <queue>
#include <deque>
#include <future>
#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"
#include "TTUtilsNamedPipe.hpp"
#include "TTUtilsOutputStream.hpp"

class TTTextBox {
public:
    explicit TTTextBox(const TTTextBoxSettings& settings, const TTUtilsOutputStream& outputStream);
    virtual ~TTTextBox();
    TTTextBox(const TTTextBox&) = delete;
    TTTextBox(const TTTextBox&&) = delete;
    TTTextBox operator=(const TTTextBox&) = delete;
    TTTextBox operator=(const TTTextBox&&) = delete;
    virtual void run();
    // Stops applications
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
private:
    // Sends heartbeat periodically and main data
    void main(std::promise<void> promise);
    void send(const char* cbegin, const char* cend);
    // IPC communication
    std::shared_ptr<TTUtilsNamedPipe> mPipe;
    // Output stream
    const TTUtilsOutputStream& mOutputStream;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTTextBoxMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};

