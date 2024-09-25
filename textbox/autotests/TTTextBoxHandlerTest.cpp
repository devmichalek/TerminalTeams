#include "TTTextBoxHandler.hpp"
#include "TTDiagnosticsLogger.hpp"
#include "TTTextBoxSettings.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <signal.h>

// Handler
std::unique_ptr<TTTextBoxHandler> handler;
// Logger
LOG_DECLARE("tteams-textbox-handler");

void messageSent(const std::string& msg) {
    std::cout << msg << std::endl;
}

void contactsSwitch(size_t id) {
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
        const TTTextBoxSettings settings(argc, argv);
        handler = std::make_unique<TTTextBoxHandler>(settings, &messageSent, &contactsSwitch);
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