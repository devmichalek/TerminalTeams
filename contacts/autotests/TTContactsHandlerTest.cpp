#include "TTContactsHandler.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <iostream>
#include <vector>
#include <signal.h>

// Logger
LOG_DECLARE("tteams-contacts-handler");

std::atomic<bool> quitHandle{false};
void signalInterruptHandler(int) {
    quitHandle.store(true);
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
        TTContactsSettings settings(argc, argv);
        TTContactsHandler handler(settings);
        LOG_INFO("Contacts handler initialized");
        while (!quitHandle.load()) {
            std::vector<std::string> tokens;
            {
                std::string line;
                std::getline(std::cin, line);
                if (quitHandle.load()) {
                    break;
                }
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

            bool status = false;
            if (tokens.size() == 2) {
                const std::string& command = tokens[0];
                if (command == "create") {
                    status = handler.create(tokens[1], "", "");
                } else {
                    auto id = std::stoi(tokens[1]);
                    if (command == "send") {
                        status = handler.send(id);
                    } else if (command == "receive") {
                        status = handler.receive(id);
                    } else if (command == "activate") {
                        status = handler.activate(id);
                    } else if (command == "deactivate") {
                        status = handler.deactivate(id);
                    } else if (command == "select") {
                        status = handler.select(id);
                    }
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