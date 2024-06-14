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
    // Main loop
    virtual void run();
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Greet message handler
    virtual bool handleGreet(const TTGreetMessage& message);
    // Heartbeat message handler
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message);
    // Returns root nickname
    virtual std::string getNickname() const;
    // Returns root identity
    virtual std::string getIdentity() const;
    // Returns root IP address and port
    virtual std::string getIpAddressAndPort() const;
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
