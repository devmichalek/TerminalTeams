#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTBroadcasterDiscoveryIf.hpp"
#include "TTTimestamp.hpp"
#include "TerminalTeams.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using tt::NeighborsDiscovery;
using tt::GreetRequest;
using tt::GreetReply;
using tt::HeartbeatRequest;
using tt::HeartbeatReply;

class TTBroadcasterDiscovery : public TTBroadcasterDiscoveryIf {
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
    virtual void run(const size_t neighborOffset);
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Greet message handler
    virtual bool handleGreet(const TTGreetMessage& message) override;
    // Heartbeat message handler
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message) override;
    // Returns root nickname
    virtual std::string getNickname() const override;
    // Returns root identity
    virtual std::string getIdentity() const override;
    // Returns root IP address and port
    virtual std::string getIpAddressAndPort() const override;
private:
    using UniqueStub = std::unique_ptr<NeighborsDiscovery::Stub>;
    UniqueStub createStub(const std::string& ipAddressAndPort);
    std::optional<TTGreetMessage> sendGreet(UniqueStub& stub);
    std::optional<TTGreetMessage> sendGreet(const std::string& ipAddressAndPort);
    std::optional<TTHeartbeatMessage> sendHeartbeat(UniqueStub& stub);
    std::optional<TTHeartbeatMessage> sendHeartbeat(const std::string& ipAddressAndPort);
    bool addNeighbor(const TTGreetMessage& message);
    size_t getNeighborsCount() const;
    struct Neighbor {
        Neighbor(TTTimestamp timestamp, size_t trials, UniqueStub stub) :
            timestamp(timestamp), trials(trials), stub(std::move(stub)) {} 
        TTTimestamp timestamp;
        size_t trials;
        UniqueStub stub;
        std::atomic<bool> locked;
    };
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    TTNetworkInterface mInterface;
    std::deque<std::string> mStaticNeighbors;
    std::deque<Neighbor> mDynamicNeighbors;
    mutable std::shared_mutex mNeighborMutex;
    static inline std::chrono::milliseconds TIMESTAMP_TIMEOUT{3000};
    static inline size_t TIMESTAMP_TRIALS{5};
};
