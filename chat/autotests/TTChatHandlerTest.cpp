#include "TTChatHandler.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <signal.h>

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
        // Signal handling
        struct sigaction signalAction;
        memset(&signalAction, 0, sizeof(signalAction));
        signalAction.sa_handler = signalInterruptHandler;
        sigfillset(&signalAction.sa_mask);
        sigaction(SIGINT, &signalAction, nullptr);
        sigaction(SIGTERM, &signalAction, nullptr);
        sigaction(SIGSTOP, &signalAction, nullptr);
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
            bool status = false;
            if (!tokens.empty()) {
                const std::string& command = tokens[0];
                const auto id = std::stoi(tokens[1]);
                // Send command
                const std::chrono::time_point<std::chrono::system_clock> dt(std::chrono::milliseconds(0));
                if (command == "send") {
                    status = handler.send(id, tokens[2], dt);
                } else if (command == "receive") {
                    status = handler.receive(id, tokens[2], dt);
                } else if (command == "select") {
                    status = handler.select(id);
                } else if (command == "create") {
                    status = handler.create(id);
                }
            }
            if (!status) {
                break;
            }
        }
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    LOG_INFO("Successfully flushed all logs");
    return 0;
}