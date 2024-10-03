#pragma once
#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"
#include "TTUtilsNamedPipe.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTUtilsInputStream.hpp"
#include "TTUtilsStopable.hpp"
#include <queue>
#include <deque>
#include <future>

class TTTextBox : public TTUtilsStopable {
public:
    explicit TTTextBox(const TTTextBoxSettings& settings,
        TTUtilsOutputStream& outputStream,
        TTUtilsInputStream& inputStream);
    virtual ~TTTextBox();
    TTTextBox(const TTTextBox&) = delete;
    TTTextBox(TTTextBox&&) = delete;
    TTTextBox& operator=(const TTTextBox&) = delete;
    TTTextBox& operator=(TTTextBox&&) = delete;
    void wait();
protected:
    TTTextBox() = default;
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
    // Sends goodbye message
    void sendGoodbye();
    // Asynchronous read
    void asynchronousRead();
    // Literals
    inline const static long QUEUED_MSG_TIMEOUT_MS = 500;
    // IPC communication
    std::shared_ptr<TTUtilsNamedPipe> mPipe;
    // Output/input stream
    TTUtilsOutputStream& mOutputStream;
    TTUtilsInputStream& mInputStream;
    // Thread concurrent message communication
    std::queue<std::unique_ptr<TTTextBoxMessage>> mQueuedMessages;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::deque<std::thread> mThreads;
    std::deque<std::future<void>> mBlockers;
    std::mutex mWaitMutex;
    std::condition_variable mWaitCondition;
};
