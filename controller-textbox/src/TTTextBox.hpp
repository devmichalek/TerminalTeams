#pragma once
#include <queue>
#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"

class TTTextBox {
public:
    explicit TTTextBox(const TTTextBoxSettings& settings);
    ~TTTextBox();
    void run();
    void stop();
private:
    // Receives hearbeat
    void heartbeat(std::promise<void> promise);
    // Sends heartbeat periodically and main data
    void main(std::promise<void> promise);
    // IPC communication
    int mNamedPipeDescriptor;
    int mSocketDescriptor;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTTextBoxMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::atomic<bool> mStopped;
    std::queue<std::thread> mThreads;
    std::queue<std::future<void>> mBlockers;
};

