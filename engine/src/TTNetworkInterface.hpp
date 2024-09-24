#pragma once
#include <string>

class TTNetworkInterface {
public:
    TTNetworkInterface(const std::string& name, const std::string& ipAddress, const std::string& port) :
        mName(name), mIpAddress(ipAddress), mPort(port) {}
    TTNetworkInterface() = default;
    virtual ~TTNetworkInterface() = default;
    TTNetworkInterface(const TTNetworkInterface&) = default;
    TTNetworkInterface(TTNetworkInterface&&) = default;
    TTNetworkInterface& operator=(const TTNetworkInterface&) = default;
    TTNetworkInterface& operator=(TTNetworkInterface&&) = default;
    [[nodiscard]] virtual std::string getName() const { return mName; }
    [[nodiscard]] virtual std::string getIpAddress() const { return mIpAddress; }
    [[nodiscard]] virtual std::string getPort() const { return mPort; }
    [[nodiscard]] virtual std::string getIpAddressAndPort() const { return mIpAddress + ":" + mPort; }
private:
    std::string mName;
    std::string mIpAddress;
    std::string mPort;
};