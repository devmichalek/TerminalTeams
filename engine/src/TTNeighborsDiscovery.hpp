#pragma once
#include <string>

struct TTGreetMessage {
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTHeartbeatMessage {
    std::string identity;
};

class TTNeighborsDiscovery {
public:
    TTNeighborsDiscovery() = default;
    virtual ~TTNeighborsDiscovery() = default;
    TTNeighborsDiscovery(const TTNeighborsDiscovery&) = default;
    TTNeighborsDiscovery(TTNeighborsDiscovery&&) = default;
    TTNeighborsDiscovery& operator=(const TTNeighborsDiscovery&) = default;
    TTNeighborsDiscovery& operator=(TTNeighborsDiscovery&&) = default;
    virtual bool handleGreet(const TTGreetMessage& message) = 0;
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message) = 0;
    virtual std::string getNickname() const = 0;
    virtual std::string getIdentity() const = 0;
    virtual std::string getIpAddressAndPort() const = 0;
};
