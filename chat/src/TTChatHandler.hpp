#pragma once
#include "TTChatMessage.hpp"
#include "TTChatSettings.hpp"
#include "TTChatEntry.hpp"
#include "TTUtilsStopable.hpp"
#include <memory>
#include <future>
#include <queue>
#include <vector>
#include <list>
#include <shared_mutex>
#include <optional>

// Class meant to be embedded into other higher abstract class.
// Allows to control TTChat process concurrently.
class TTChatHandler : public TTUtilsStopable {
public:
    explicit TTChatHandler(const TTChatSettings& settings);
    virtual ~TTChatHandler();
    TTChatHandler(const TTChatHandler&) = delete;
    TTChatHandler(TTChatHandler&&) = delete;
    TTChatHandler& operator=(const TTChatHandler&) = delete;
    TTChatHandler& operator=(TTChatHandler&&) = delete;
    virtual bool send(size_t id, const std::string& message, TTChatTimestamp timestamp);
    virtual bool receive(size_t id, const std::string& message, TTChatTimestamp timestamp);
    virtual bool select(size_t id);
    virtual bool create(size_t id);
    virtual bool size() const;
    [[nodiscard]] virtual std::optional<TTChatEntries> get(size_t id) const;
    [[nodiscard]] virtual std::optional<size_t> current() const;
protected:
    TTChatHandler() = default;
    virtual void onStop() override;
private:
    bool send(TTChatMessageType type, const std::string& data, TTChatTimestamp timestamp);
    std::list<std::unique_ptr<TTChatMessage>> dequeue();
    // Receives heartbeat
    void heartbeat();
    // Sends heartbeat periodically or main data if available
    void main();
    // Sends last bit of information - goodbye message
    void goodbye();
    // IPC message queue communication
    std::shared_ptr<TTUtilsMessageQueue> mPrimaryMessageQueue;
    std::shared_ptr<TTUtilsMessageQueue> mSecondaryMessageQueue;
    static inline const std::chrono::milliseconds mHeartbeatTimeout{500};
    // Thread concurrent message communication
    std::future<void> mHeartbeatResult;
    std::future<void> mHandlerResult;
    std::thread mHeartbeatThread;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::queue<std::unique_ptr<TTChatMessage>> mQueuedMessages;
    // Messages storage
    std::optional<size_t> mCurrentId;
    mutable std::shared_mutex mMessagesMutex;
    std::vector<TTChatEntries> mMessages;
};