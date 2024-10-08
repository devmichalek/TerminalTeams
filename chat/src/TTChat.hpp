#pragma once
#include "TTChatSettings.hpp"
#include "TTChatMessage.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTUtilsStopable.hpp"
#include <future>
#include <functional>
#include <deque>

class TTChat : public TTUtilsStopable {
public:
    explicit TTChat(const TTChatSettings& settings, TTUtilsOutputStream& outputStream);
    virtual ~TTChat();
    TTChat(const TTChat&) = delete;
    TTChat(TTChat&&) = delete;
    TTChat& operator=(const TTChat&) = delete;
    TTChat& operator=(TTChat&&) = delete;
    // Receives main data
    virtual void run();
protected:
    TTChat() = default;
private:
    // Sends heartbeat periodically
    void heartbeat(std::promise<void> promise);
    // Handles all message types
    bool handle(const TTChatMessage& message);
    // Prints message in a defined format
    void print(TTChatMessageType type, TTChatTimestamp timestamp, std::string data);
    // IPC message queue communication
    std::shared_ptr<TTUtilsMessageQueue> mPrimaryMessageQueue;
    std::shared_ptr<TTUtilsMessageQueue> mSecondaryMessageQueue;
    static inline const std::chrono::milliseconds mHeartbeatTimeout{500};
    // Thread concurrent message communication
    std::future<void> mHeartbeatResult;
    std::thread mHeartbeatThread;
    // Terminal data
    size_t mWidth;
    size_t mHeight;
    size_t mSideWidth;
    std::string mBlankLine;
    // Output stream
    TTUtilsOutputStream& mOutputStream;
    // Gathered chunks
    std::deque<TTChatMessage> mChunks;
};
