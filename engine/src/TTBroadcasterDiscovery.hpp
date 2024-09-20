#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTUtilsTimerFactory.hpp"
#include "TTNeighborsStub.hpp"

class TTBroadcasterDiscovery {
public:
    TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                           TTChatHandler& chatHandler,
                           TTNeighborsStub& neighborsStub,
                           TTNetworkInterface networkInterface,
                           const std::deque<std::string>& neighbors);
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
    virtual std::string getNickname();
    // Returns root identity
    virtual std::string getIdentity();
    // Returns root IP address and port
    virtual std::string getIpAddressAndPort();
private:
    void resolveStaticNeighbors();
    void resolveDynamicNeighbors();
    bool addNeighbor(const std::string& nickname,
        const std::string& identity,
        const std::string& ipAddressAndPort,
        TTUniqueDiscoveryStub stub);
    struct StaticNeighbor {
        StaticNeighbor(TTUtilsTimer timer, std::string ipAddressAndPort) :
            timer(timer), trials(discoveryTrials), ipAddressAndPort(ipAddressAndPort) {}
        ~StaticNeighbor() = default;
        StaticNeighbor(const StaticNeighbor&) = default;
        StaticNeighbor(StaticNeighbor&&) = default;
        StaticNeighbor& operator=(const StaticNeighbor&) = default;
        StaticNeighbor& operator=(StaticNeighbor&&) = default;
        TTUtilsTimer timer;
        size_t trials;
        std::string ipAddressAndPort;
        const static inline size_t discoveryTrials = 3;
    };
    struct DynamicNeighbor {
        DynamicNeighbor(TTUtilsTimer timer, TTUniqueDiscoveryStub stub) :
            timer(timer), trials(inactivityTrials), stub(std::move(stub)) {}
        ~DynamicNeighbor() = default;
        DynamicNeighbor(const DynamicNeighbor&) = default;
        DynamicNeighbor(DynamicNeighbor&&) = default;
        DynamicNeighbor& operator=(const DynamicNeighbor&) = default;
        DynamicNeighbor& operator=(DynamicNeighbor&&) = default;
        TTUtilsTimer timer;
        size_t trials;
        TTUniqueDiscoveryStub stub;
        const static inline size_t inactivityTrials = 5;
    };
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    TTNeighborsStub& mNeighborsStub;
    TTNetworkInterface mNetworkInterface;
    std::deque<StaticNeighbor> mStaticNeighbors;
    std::map<size_t, DynamicNeighbor> mDynamicNeighbors;
    mutable std::shared_mutex mNeighborMutex;
    TTUtilsTimerFactory mInactivityTimerFactory;
    TTUtilsTimerFactory mDiscoveryTimerFactory;
};
