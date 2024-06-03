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
    virtual ~TTNeighborsDiscovery();
    virtual bool handleGreet(const TTGreetMessage& message) = 0;
    virtual bool handleHeartbeat(const TTHeartbeatMessage& message) = 0;
    virtual std::string getNickname() = 0;
    virtual std::string getIdentity() = 0;
    virtual std::string getIpAddressAndPort() = 0;
protected:
    TTNeighborsDiscovery() = default;
    TTNeighborsDiscovery(const TTNeighborsDiscovery&) = delete;
    TTNeighborsDiscovery(TTNeighborsDiscovery&&) = delete;
    TTNeighborsDiscovery operator=(const TTNeighborsDiscovery&) = delete;
    TTNeighborsDiscovery operator=(TTNeighborsDiscovery&&) = delete;
};
