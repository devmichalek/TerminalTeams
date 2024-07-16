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
    TTTextBox(TTTextBox&&) = delete;
    TTTextBox& operator=(const TTTextBox&) = delete;
    TTTextBox& operator=(TTTextBox&&) = delete;
    virtual void run();
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
private:
    // Parses input and returns true if there are no suspicions
    bool parse(const std::string& line);
    // Executes command
    bool execute(const std::vector<std::string>& args);
    // Sends casual message
    bool send(const char* cbegin, const char* cend);
    // Sends heartbeat periodically and main data
    void main(std::promise<void> promise);
    // Generic queue
    void queue(std::unique_ptr<TTTextBoxMessage> message);
    // IPC communication
    std::shared_ptr<TTUtilsNamedPipe> mPipe;
    // Output stream
    TTUtilsOutputStream& mOutputStream;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTTextBoxMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::atomic<bool> mStopped;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
};

