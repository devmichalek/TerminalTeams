#include "TTEngineSettings.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>

TTEngineSettings::TTEngineSettings(int argc, char** argv) {
    const std::string classNamePrefix = "TTEngineSettings: ";
    if (argc < MIN_ARGC) {
        throw std::runtime_error(classNamePrefix + "insufficient number of arguments");
    }

    mContactsSharedName = std::string(argv[1]);
    mChatQueueName = std::string(argv[2]);
    mTextboxPipeName = std::string(argv[3]);

    mInterface = argv[4];

    mIpAddress = argv[5];
    sockaddr_in sa;
    if (inet_pton(AF_INET, mIpAddress.c_str(), &(sa.sin_addr)) == 0) {
        throw std::runtime_error(classNamePrefix + "invalid IPv4 address=" + mIpAddress);
    }

    const auto portStr = std::string(argv[6]);
    ss.str(portStr);
    ss.clear();
    ss >> mPort;
    if (ss.fail()) {
        throw std::runtime_error(classNamePrefix + "invalid port=" + portStr);
    }

    for (int i = MIN_ARGC; i < argc; ++i) {
        std::string neighbor = argv[i];
        sockaddr_in sa;
        if (inet_pton(AF_INET, neighbor.c_str(), &(sa.sin_addr)) == 0) {
            throw std::runtime_error(classNamePrefix + "invalid neighbor IPv4 address=" + neighbor);
        }
        mNeighbors.emplace_back(mNeighbors);
    }
}
