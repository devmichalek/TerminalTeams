#pragma once
#include "TTChatSettings.hpp"
#include "TTChatMessage.hpp"
#include "TTUtilsOutputStream.hpp"
#include <future>
#include <functional>

class TTChat {
public:
    explicit TTChat(const TTChatSettings& settings, const TTUtilsOutputStream& outputStream);
    virtual ~TTChat();
    TTChat(const TTChat&) = delete;
    TTChat(TTChat&&) = delete;
    TTChat operator=(const TTChat&) = delete;
    TTChat operator=(TTChat&&) = delete;
    // Receives main data
    virtual void run();
    // Stops applications
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
private:
    // Sends heartbeat periodically
    void heartbeat(std::promise<void> promise);
    // Handles all message types
    void handle(const TTChatMessage& message);
    // Prints message in a defined format
    void print(const char* cmessage, TTChatTimestamp timestmap, bool received);
    // IPC message queue communication
    std::shared_ptr<TTUtilsMessageQueue> mPrimaryMessageQueue;
    std::shared_ptr<TTUtilsMessageQueue> mSecondaryMessageQueue;
    // Thread concurrent message communication
    std::atomic<bool> mStopped;
    std::future<void> mHeartbeatResult;
    std::thread mHeartbeatThread;
    // Terminal data
    size_t mWidth;
    size_t mHeight;
    size_t mSideWidth;
    std::string mBlankLine;
    // Output stream
    const TTUtilsOutputStream& mOutputStream;
};
