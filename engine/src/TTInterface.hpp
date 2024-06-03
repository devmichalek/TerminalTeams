#pragma once
#include <string>

class TTInterface {
public:
    TTInterface(std::string name, std::string ipAddress, std::string port);
    virtual ~TTInterface();
    TTInterface(const TTInterface&) = default;
    TTInterface(TTInterface&&) = default;
    TTInterface operator=(const TTInterface&) = default;
    TTInterface operator=(TTInterface&&) = default;
    std::string getName() const { return mName; }
    std::string getIpAddress() const { return mIpAddress; }
    std::string getPort() const { return mPort; }
    std::string getIpAddressAndPort() const { return mIpAddress + ":" + mPort; }
private:
    std::string mName;
    std::string mIpAddress;
    std::string mPort;
};