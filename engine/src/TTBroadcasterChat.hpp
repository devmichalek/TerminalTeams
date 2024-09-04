#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTNeighborsStub.hpp"
#include "TTTimestamp.hpp"
#include <random>

class TTBroadcasterChat {
public:
    TTBroadcasterChat(TTContactsHandler& contactsHandler,
                      TTChatHandler& chatHandler,
                      TTNeighborsStub& neighborsStub,
                      TTNetworkInterface interface);
    virtual ~TTBroadcasterChat();
    TTBroadcasterChat(const TTBroadcasterChat&) = delete;
    TTBroadcasterChat(TTBroadcasterChat&&) = delete;
    TTBroadcasterChat& operator=(const TTBroadcasterChat&) = delete;
    TTBroadcasterChat& operator=(TTBroadcasterChat&&) = delete;
    // Main loop
    virtual void run();
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Handles message (send)
    virtual bool handleSend(const std::string& message);
    // Handles request (receive)
    virtual bool handleReceive(const TTTellRequest& request);
    // Handles request (receive)
    virtual bool handleReceive(const TTNarrateRequest& request);
    // Returns root nickname
    virtual std::string getIdentity();
private:
    struct Neighbor {
        Neighbor() = default;
        ~Neighbor() = default;
        Neighbor(const Neighbor&) = default;
        Neighbor(Neighbor&&) = default;
        Neighbor& operator=(const Neighbor&) = default;
        Neighbor& operator=(Neighbor&&) = default;
        TTUniqueChatStub stub;
        TTTimestamp timestamp;
        std::deque<std::string> pendingMessages;
    };
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    TTNeighborsStub& mNeighborsStub;
    TTNetworkInterface mInterface;
    std::mutex mNeighborsMutex;
    std::condition_variable mNeighborsCondition;
    std::map<size_t, Neighbor> mNeighbors;
    std::atomic<bool> mNeighborsFlag;
    static inline const size_t NEIGHBORS_FLAG_TIMEOUT{500};
    static inline const size_t INACTIVITY_TIMEOUT_MIN{3000};
    static inline const size_t INACTIVITY_TIMEOUT_MAX{6000};
    std::random_device mRandomDevice;
    std::mt19937 mRandomNumberGenerator;
    std::uniform_int_distribution<std::mt19937::result_type> mInactivityDistribution;
};
