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
        args.push_back(argv[4]);
        args.push_back(argv[5]);
        args.push_back(argv[7]);
        args.push_back(argv[8]);
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

    {
        std::string name = argv[6];
        std::string ipAddress = argv[7];
        sockaddr_in sa;
        if (inet_pton(AF_INET, mIpAddress.c_str(), &(sa.sin_addr)) == 0) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid IPv4 address=") + mIpAddress);
        }
        std::string port = argv[8];
        mInterface = TTInterface(name, ipAddress, port);
    }


    for (int i = MIN_ARGC; i < argc; ++i) {
        std::string neighbor = argv[i];
        sockaddr_in sa;
        if (inet_pton(AF_INET, neighbor.c_str(), &(sa.sin_addr)) == 0) {
            throw std::runtime_error(std::string("TTEngineSettings: Invalid neighbor IPv4 address=") + neighbor);
        }
        mNeighbors.emplace_back(neighbor);
    }
}
