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

std::vector<std::string> getTokens(std::string line, const std::string delimiter) {
    std::vector<std::string> tokens;
    // Command
    size_t pos = line.find(delimiter);
    if (pos == std::string::npos) {
        throw std::runtime_error("TTChatHandlerTest: Failed to read command");
    }
    std::string token = line.substr(0, pos);
    tokens.push_back(token);
    line.erase(0, pos + delimiter.length());
    // ID
    pos = line.find(delimiter);
    if (pos == std::string::npos) {
        throw std::runtime_error("TTChatHandlerTest: Failed to read ID");
    }
    token = line.substr(0, pos);
    tokens.push_back(token);
    line.erase(0, pos + delimiter.length());
    // The rest
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

    std::cout << "1.\n";

	// Run main app
    TTChatHandler handler(queueMessageName, &sent, &received);
    std::cout << "2.\n";
    while (!quitHandle.load()) {
        std::cout << "3.\n";
        // Get tokens
        std::string line;
        std::getline(std::cin, line);
        const std::string delimiter = " ";
        auto tokens = getTokens(line, delimiter);
        const std::string& command = tokens[0];
        const auto id = std::stoi(tokens[1]);
        // Send command
        bool status = false;
        if (command == "send") {
            status = handler.send(id, tokens[2], std::chrono::system_clock::now());
        } else if (command == "receive") {
            status = handler.receive(id, tokens[2], std::chrono::system_clock::now());
        } else if (command == "clear") {
            status = handler.clear(id);
        }
        if (!status) {
            break;
        }
    }

	return 0;
}