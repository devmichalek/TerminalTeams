#include "TTEngineSettings.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>

TTEngineSettings::TTEngineSettings(int argc, const char* const* argv) {
    if (argc < MIN_ARGC) {
        throw std::runtime_error("TTEngineSettings: Insufficient number of arguments");
    }

    // Set up contacts settings
    {
        std::vector<const char*> args;
        args.push_back(argv[0]);
        args.push_back("0");
        args.push_back("0");
        args.push_back(argv[1]);
        mContactsSettings = std::make_unique<TTContactsSettings>(args.size(), args.data());
    }

    // Set up chat settings
    {
        std::vector<const char*> args;
        args.push_back(argv[0]);
        args.push_back("0");
        args.push_back("0");
        args.push_back(argv[2]);
        mChatSettings = std::make_unique<TTChatSettings>(args.size(), args.data());
    }

    // Set up textbox settings
    {
        std::vector<const char*> args;
        args.push_back(argv[0]);
        args.push_back("0");
        args.push_back("0");
        args.push_back(argv[2]);
        mTextBoxSettings = std::make_unique<TTTextBoxSettings>(args.size(), args.data());
    }

    mInterface = argv[4];

    mIpAddress = argv[5];
    sockaddr_in sa;
    if (inet_pton(AF_INET, mIpAddress.c_str(), &(sa.sin_addr)) == 0) {
        throw std::runtime_error(std::string("TTEngineSettings: Invalid IPv4 address=") + mIpAddress);
    }

    const auto portStr = std::string(argv[6]);
    ss.str(portStr);
    ss.clear();
    ss >> mPort;
    if (ss.fail()) {
        throw std::runtime_error(std::string("TTEngineSettings: Invalid port=") + portStr);
    }

    for (int i = MIN_ARGC; i < argc; ++i) {
        std::string neighbor = argv[i];
        sockaddr_in sa;
        if (inet_pton(AF_INET, neighbor.c_str(), &(sa.sin_addr)) == 0) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid neighbor IPv4 address=") + neighbor);
        }
        mNeighbors.emplace_back(mNeighbors);
    }
}
