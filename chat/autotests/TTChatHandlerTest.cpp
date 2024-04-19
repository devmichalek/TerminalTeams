#include "TTChatHandler.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <signal.h>

std::atomic<size_t> sentCounter{0};
void sent() {
    sentCounter++;
}

std::atomic<size_t> receivedCounter{0};
void received() {
    receivedCounter++;
}

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
    if (argc != 2) {
        throw std::runtime_error("invalid number of arguments");
    }
    std::string queueMessageName = argv[1];

    // Signal handling
    struct sigaction signalAction;
    memset(&signalAction, 0, sizeof(signalAction));
    signalAction.sa_handler = signalInterruptHandler;
    sigfillset(&signalAction.sa_mask);
    sigaction(SIGINT, &signalAction, nullptr);
    sigaction(SIGTERM, &signalAction, nullptr);
    sigaction(SIGSTOP, &signalAction, nullptr);

    // Run main app
    TTChatHandler handler(queueMessageName, &sent, &received);
    while (!quitHandle.load()) {
        // Get tokens
        std::string line;
        std::getline(std::cin, line);
        if (quitHandle.load()) {
            break;
        }
        auto tokens = getTokens(line);
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
            } else if (command == "clear") {
                status = handler.clear(id);
            } else if (command == "create") {
                status = handler.create(id);
            }
        }
        if (!status) {
            break;
        }
    }

    return 0;
}