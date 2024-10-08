#include "TTEngineSettings.hpp"
#include <string>
#include <limits>
#include <charconv>
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
        args.push_back(argv[3]);
        mTextBoxSettings = std::make_unique<TTTextBoxSettings>(args.size(), args.data());
    }

    // Set up abstract factory
    mAbstractFactory = std::make_unique<TTAbstractFactory>(*mContactsSettings, *mChatSettings, *mTextBoxSettings);

    {
        mNickname = argv[4];
        mIdentity = argv[5];
    }

    {
        const std::string name = argv[6];
        const std::string ipAddress = argv[7];
        sockaddr_in sa;
        if (inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr)) == 0) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid IPv4 address=") + ipAddress);
        }
        size_t port = -1;
        auto [ptr, ec] = std::from_chars(argv[8], argv[8] + strlen(argv[8]), port);
        if (ec != std::errc()) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid port=") + argv[8]);
        }
        if (port > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid (out of range) port=") + argv[8]);
        }
        mNetworkInterface = TTNetworkInterface(name, ipAddress, argv[8]);
    }

    for (int i = MIN_ARGC; i < argc; ++i) {
        const std::string neighbor = argv[i];
        sockaddr_in sa;
        if (inet_pton(AF_INET, neighbor.c_str(), &(sa.sin_addr)) == 0) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid neighbor IPv4 address=") + neighbor);
        }
        mNeighbors.emplace_back(neighbor);
    }
}
