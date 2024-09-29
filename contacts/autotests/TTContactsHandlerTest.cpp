#include "TTContactsHandler.hpp"
#include "TTUtilsSignals.hpp"
#include <iostream>
#include <vector>

// Logger
LOG_DECLARE("tteams-contacts-handler");

std::atomic<bool> quitHandle{false};
void signalInterruptHandler(int) {
    quitHandle.store(true);
}

int main(int argc, char** argv) {
    try {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.setup(signalInterruptHandler, { SIGINT, SIGTERM, SIGSTOP });
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

            if (tokens.size() == 2) [[likely]] {
                const std::string& command = tokens[0];
                if (command == "create") {
                    handler.create(tokens[1], "", "");
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
                    }
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