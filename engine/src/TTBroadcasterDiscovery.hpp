#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTTimestamp.hpp"
#include "TTBroadcasterStub.hpp"

class TTBroadcasterDiscovery {
public:
    TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                           TTChatHandler& chatHandler,
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
    // Greet request handler
    virtual bool handleGreet(const TTGreetRequest& request);
    // Heartbeat request handler
    virtual bool handleHeartbeat(const TTHeartbeatRequest& request);
    // Returns root nickname
    virtual std::string getNickname() const;
    // Returns root identity
    virtual std::string getIdentity() const;
    // Returns root IP address and port
    virtual std::string getIpAddressAndPort() const;
private:
    bool addNeighbor(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort);
    struct Neighbor {
        Neighbor(TTTimestamp timestamp, size_t trials, UniqueDiscoveryStub stub) :
            timestamp(timestamp), trials(trials), stub(std::move(stub)) {} 
        TTTimestamp timestamp;
        size_t trials;
        UniqueDiscoveryStub stub;
    };
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    TTNetworkInterface mInterface;
    TTBroadcasterStub mBroadcasterStub;
    std::deque<std::string> mStaticNeighbors;
    std::map<size_t, Neighbor> mDynamicNeighbors;
    mutable std::shared_mutex mNeighborMutex;
    static inline std::chrono::milliseconds TIMESTAMP_TIMEOUT{3000};
    static inline size_t TIMESTAMP_TRIALS{5};
};
