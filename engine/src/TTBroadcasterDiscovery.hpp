#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTNeighborsDiscovery.hpp"
#include "TTTimestamp.hpp"

class TTBroadcasterDiscovery : public TTNeighborsDiscovery {
public:
    TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                           TTNetworkInterface interface,
                           std::deque<std::string> neighbors);
    virtual ~TTBroadcasterDiscovery();
    TTBroadcasterDiscovery(const TTBroadcasterDiscovery&) = delete;
    TTBroadcasterDiscovery(TTBroadcasterDiscovery&&) = delete;
    TTBroadcasterDiscovery& operator=(const TTBroadcasterDiscovery&) = delete;
    TTBroadcasterDiscovery& operator=(TTBroadcasterDiscovery&&) = delete;
    virtual void run();
    virtual void stop();
    virtual bool stopped() const;
    virtual bool handleGreet(const TTGreetMessage& message);
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message);
    virtual std::string getNickname();
    virtual std::string getIdentity();
    virtual std::string getIpAddressAndPort();
private:
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTNetworkInterface mInterface;
    std::deque<std::string> mNeighbors;
    std::deque<TTTimestamp> mTimestamps;
    std::deque<size_t> mTimestampTrials;
    static inline std::chrono::milliseconds SLOW_START_THRESHOLD{5000};
    static inline std::chrono::milliseconds FAST_RECOVERY_THRESHOLD{1000};
    static inline size_t SLOW_START_TRIALS{6};
    static inline size_t FAST_RECOVERY_TRIALS{3};
};
