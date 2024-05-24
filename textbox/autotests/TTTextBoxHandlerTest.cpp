#include "TTTextBoxHandler.hpp"
#include "TTDiagnosticsLogger.hpp"
#include "TTTextBoxSettings.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <signal.h>

std::unique_ptr<TTTextBoxHandler> handler;
const std::string LOGGER_PREFIX = "Main:";
TTDiagnosticsLogger TTDiagnosticsLogger::mInstance("tteams-textbox-handler");

void messageSent(std::string msg) {
    std::cout << msg << std::endl;
}

void contactsSwitch(size_t id) {
    std::cout << '#' << id << std::endl;
}

void signalInterruptHandler(int) {
    if (handler) {
        TTDiagnosticsLogger::getInstance().warning("{} Stopping due to caught signal", LOGGER_PREFIX);
        handler->stop();
    }
}

int main(int argc, char** argv) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    try {
        // Signal handling
        struct sigaction signalAction;
        memset(&signalAction, 0, sizeof(signalAction));
        signalAction.sa_handler = signalInterruptHandler;
        sigfillset(&signalAction.sa_mask);
        sigaction(SIGINT, &signalAction, nullptr);
        sigaction(SIGTERM, &signalAction, nullptr);
        sigaction(SIGSTOP, &signalAction, nullptr);
        logger.info("{} Signal handling initialized", LOGGER_PREFIX);
        // Run main app
        const TTTextBoxSettings settings(argc, argv);
        handler = std::make_unique<TTTextBoxHandler>(settings, &messageSent, &contactsSwitch);
        logger.info("{} Textbox handler initialized", LOGGER_PREFIX);
        while (!handler->stopped()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& exp) {
        logger.info("{} Exception captured: {}", LOGGER_PREFIX, exp.what());
    }
    logger.info("{} Successfully flushed all logs", LOGGER_PREFIX);
    return 0;
}