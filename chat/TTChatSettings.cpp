#include "TTChatSettings.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>

TTChatSettings::TTChatSettings(int argc, char** argv) {
    const std::string classNamePrefix = "TTChatSettings: ";
    if (argc != MAX_ARGC) {
        throw std::runtime_error(classNamePrefix + "invalid number of arguments");
    }

    const auto widthStr = std::string(argv[1]);
    std::stringstream ss(widthStr);
    ss >> mWidth;
    if (ss.fail()) {
        throw std::runtime_error(classNamePrefix + "invalid terminal emulator width=" + widthStr);
    }

    const auto heightStr = std::string(argv[2]);
    ss.str(heightStr);
    ss.clear();
    ss >> mHeight;
    if (ss.fail()) {
        throw std::runtime_error(classNamePrefix + "invalid terminal emulator height=" + heightStr);
    }

    mInterface = argv[3];

    mIpAddress = argv[4];
    sockaddr_in sa;
    if (inet_pton(AF_INET, mIpAddress.c_str(), &(sa.sin_addr)) == 0) {
        throw std::runtime_error(classNamePrefix + "invalid IPv4 address=" + mIpAddress);
    }

    const auto portStr = std::string(argv[5]);
    ss.str(portStr);
    ss.clear();
    ss >> mPort;
    if (ss.fail()) {
        throw std::runtime_error(classNamePrefix + "invalid port=" + portStr);
    }
}

size_t TTChatSettings::getTerminalWidth() const {
    return mWidth;
}

size_t TTChatSettings::getTerminalHeight() const {
    return mHeight;
}

std::string TTChatSettings::getInterface() const {
    return mInterface;
}

std::string TTChatSettings::getIpAddress() const {
    return mIpAddress;
}

uint16_t TTChatSettings::getPort() const {
    return mPort;
}