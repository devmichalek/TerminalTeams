#pragma once
#include <string>

class TTChatSettings {
public:
    explicit TTChatSettings(int argc, char** argv);
    size_t getTerminalWidth() const;
    size_t getTerminalHeight() const;
    std::string getInterface() const;
    std::string getIpAddress() const;
    uint16_t getPort() const;

private:
    size_t mWidth;
    size_t mHeight;
    std::string mInterface;
    std::string mIpAddress;
    uint16_t mPort;
    static inline constexpr int MAX_ARGC = 6;
};
