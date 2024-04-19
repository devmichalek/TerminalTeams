#pragma once
#include <queue>
#include <deque>
#include <future>
#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"

class TTTextBox {
public:
    explicit TTTextBox(const TTTextBoxSettings& settings);
    ~TTTextBox();
    void run();
    void stop();
    bool stopped() const;
private:
    // Sends heartbeat periodically and main data
    void main(std::promise<void> promise);
    void send(const char* cbegin, const char* cend);
    // IPC communication
    int mNamedPipeDescriptor;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTTextBoxMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};

