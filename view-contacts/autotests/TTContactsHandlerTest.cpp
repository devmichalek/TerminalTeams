#include "TTContactsHandler.hpp"
#include <iostream>
#include <vector>
#include <signal.h>

std::atomic<size_t> producedCounter{0};
void produced() {
	producedCounter++;
}

std::atomic<size_t> consumedCounter{0};
void consumed() {
	consumedCounter++;
}

std::atomic<bool> quitHandle{false};
void signalInterruptHandler(int) {
	quitHandle.store(true);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error("invalid number of arguments");
    }
    std::string sharedName = argv[1];

    // Signal handling
    struct sigaction signalAction;
	memset(&signalAction, 0, sizeof(signalAction));
    signalAction.sa_handler = signalInterruptHandler;
    sigfillset(&signalAction.sa_mask);
    sigaction(SIGINT, &signalAction, nullptr);

	// Run main app
	TTContactsHandler handler(sharedName, &produced, &consumed);
    while (!quitHandle.load()) {
        std::vector<std::string> tokens;
        {
            std::string line;
            std::getline(std::cin, line);
            size_t pos = 0;
            std::string token;
            const std::string delimiter = " ";
            while ((pos = line.find(delimiter)) != std::string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + delimiter.length());
            }
            tokens.push_back(line);
        }

        if (tokens.size() == 2) {
            const std::string& command = tokens[0];
            if (command == "create") {
                handler.create(tokens[1], "", "", "");
            } else {
                auto id = std::stoi(tokens[1]);
                if (command == "send") {
                    handler.send(id);
                } else if (command == "receive") {
                    handler.receive(id);
                } else if (command == "activate") {
                    handler.activate(id);
                } else if (command == "deactivate") {
                    handler.deactivate(id);
                } else if (command == "select") {
                    handler.select(id);
                } else if (command == "unselect") {
                    handler.unselect(id);
                }
            }
        }
    }
	return 0;
}