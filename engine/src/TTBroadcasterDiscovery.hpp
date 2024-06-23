#pragma once
#include <grpcpp/grpcpp.h>
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTNeighborsDiscovery.hpp"
#include "TTTimestamp.hpp"
#include "TerminalTeams.grpc.pb.h"

using tt::NeighborsDiscovery;
using tt::GreetRequest;
using tt::GreetReply;
using tt::HeartbeatRequest;
using tt::HeartbeatReply;

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
    virtual void run(const size_t neighborOffset);
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Greet message handler
    virtual bool handleGreet(const TTGreetMessage& message);
    // Heartbeat message handler
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message);
    // Returns root nickname
    virtual const std::string& getNickname() const;
    // Returns root identity
    virtual const std::string& getIdentity() const;
    // Returns root IP address and port
    virtual const std::string& getIpAddressAndPort() const;
private:
    std::unique_ptr<NeighborsDiscovery::Stub> createStub(const std::string& ipAddressAndPort);
    TTGreetMessage sendGreet(NeighborsDiscovery::Stub* stub);
    TTGreetMessage sendGreet(const std::string& ipAddressAndPort);
    TTHeartbeatMessage sendHeartbeat(const std::string& ipAddressAndPort);
    TTHeartbeatMessage sendHeartbeat(NeighborsDiscovery::Stub* stub);
    bool addNeighbor(const TTGreetMessage& message);
    size_t getNeighborsCount() const;
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTNetworkInterface mInterface;
    std::deque<TTTimestamp> mTimestamps;
    std::deque<size_t> mTimestampTrials;
    std::deque<std::unique_ptr<NeighborsDiscovery::Stub>> mStubs;
    mutable std::shared_mutex mNeighborMutex;
    static inline std::chrono::milliseconds TIMESTAMP_TIMEOUT{3000};
    static inline size_t TIMESTAMP_TRIALS{5};
};
