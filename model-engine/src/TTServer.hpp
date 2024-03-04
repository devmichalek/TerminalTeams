#pragma once

class TTServer {
public:
    explicit TTServer(uint32_t ipAddress, uint16_t port);
    void run();
private:
    std::string mIpAddressAndPort;
};