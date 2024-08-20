#pragma once
#include <string>

struct TTGreetMessage {
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
    unsigned int sequenceNumber;
};

struct TTHeartbeatMessage {
    std::string identity;
};

class TTBroadcasterDiscoveryIf {
public:
    TTBroadcasterDiscoveryIf() = default;
    virtual ~TTBroadcasterDiscoveryIf() = default;
    TTBroadcasterDiscoveryIf(const TTBroadcasterDiscoveryIf&) = default;
    TTBroadcasterDiscoveryIf(TTBroadcasterDiscoveryIf&&) = default;
    TTBroadcasterDiscoveryIf& operator=(const TTBroadcasterDiscoveryIf&) = default;
    TTBroadcasterDiscoveryIf& operator=(TTBroadcasterDiscoveryIf&&) = default;
    virtual bool handleGreet(const TTGreetMessage& message) = 0;
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message) = 0;
    virtual std::string getNickname() const = 0;
    virtual std::string getIdentity() const = 0;
    virtual std::string getIpAddressAndPort() const = 0;
};
