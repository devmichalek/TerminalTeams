#include "TTChatHandler.hpp"
#include "TTUtilsSignals.hpp"
#include <iostream>
#include <chrono>
#include <vector>

// Logger
LOG_DECLARE("tteams-chat-handler");

std::atomic<bool> quitHandle{false};
void signalInterruptHandler(int) {
    quitHandle.store(true);
}

std::vector<std::string> getTokens(std::string line) {
    std::vector<std::string> tokens;
    const std::string delimiter = " ";
    // Command
    size_t pos = line.find(delimiter);
    if (pos == std::string::npos) {
        return {};
    }
    std::string token = line.substr(0, pos);
    tokens.push_back(token);
    line.erase(0, pos + delimiter.length());
    // ID
    pos = line.find(delimiter);
    token = line.substr(0, pos);
    tokens.push_back(token);
    // The rest - message
    line.erase(0, pos + delimiter.length());
    if (!line.empty()) {
        tokens.push_back(line);
    }
    return tokens;
}

int main(int argc, char** argv) {
    try {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.setup(signalInterruptHandler, { SIGINT, SIGTERM, SIGSTOP });
        LOG_INFO("Signal handling initialized");
        // Run main app
        TTChatSettings settings(argc, argv);
        TTChatHandler handler(settings);
        LOG_INFO("Chat handler initialized");
        while (!quitHandle.load()) {
            // Get tokens
            std::vector<std::string> tokens;
            {
                std::string line;
                std::getline(std::cin, line);
                if (quitHandle.load()) {
                    break;
                }
                tokens = getTokens(line);
            }
            if (!tokens.empty()) [[likely]] {
                const std::string& command = tokens[0];
                const auto id = std::stoi(tokens[1]);
                // Send command
                const std::chrono::time_point<std::chrono::system_clock> dt(std::chrono::milliseconds(0));
                if (command == "send") {
                    std::cout << "send status=" << static_cast<int>(handler.send(id, tokens[2], dt)) << std::endl;
                } else if (command == "receive") {
                    std::cout << "receive status=" << static_cast<int>(handler.receive(id, tokens[2], dt)) << std::endl;
                } else if (command == "select") {
                    std::cout << "select status=" << static_cast<int>(handler.select(id)) << std::endl;
                } else if (command == "create") {
                    std::cout << "create status=" << static_cast<int>(handler.create(id)) << std::endl;
                }
            } else {
                break;
            }
        }
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    LOG_INFO("Successfully flushed all logs");
    return 0;
}