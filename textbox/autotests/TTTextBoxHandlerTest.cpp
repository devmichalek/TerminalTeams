#include "TTTextBoxHandler.hpp"
#include "TTUtilsSignals.hpp"
#include "TTTextBoxSettings.hpp"
#include <iostream>
#include <chrono>
#include <cstring>

// Handler
std::unique_ptr<TTTextBoxHandler> handler;
// Logger
LOG_DECLARE("tteams-textbox-handler");

void messageSent(const std::string& msg) {
    std::cout << msg << std::endl;
}

void contactsSelection(size_t id) {
    std::cout << '#' << id << std::endl;
}

void signalInterruptHandler(int) {
    if (handler) {
        LOG_WARNING("Stopping due to caught signal!");
        handler->stop();
    }
}

int main(int argc, char** argv) {
    try {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.setup(signalInterruptHandler, { SIGINT, SIGTERM, SIGSTOP });
        // Run main app
        const TTTextBoxSettings settings(argc, argv);
        handler = std::make_unique<TTTextBoxHandler>(settings, &messageSent, &contactsSelection);
        LOG_INFO("TextBox handler initialized");
        while (!handler->isStopped()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    handler.reset();
    LOG_INFO("Successfully flushed all logs");
    return 0;
}