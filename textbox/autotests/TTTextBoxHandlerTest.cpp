#include "TTTextBoxHandler.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <signal.h>

// Logger
TTDiagnosticsLogger TTDiagnosticsLogger::mInstance("tteams-textbox-handler");

void messageSent(std::string msg) {
    std::cout << msg << std::endl;
}

void contactsSwitch(size_t id) {
    std::cout << '#' << id << std::endl;
}

std::unique_ptr<TTTextBoxHandler> handler;
void signalInterruptHandler(int) {
    if (handler) {
        handler->stop();
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error("invalid number of arguments");
    }
    std::string uniqueName = argv[1];

    // Signal handling
    struct sigaction signalAction;
    memset(&signalAction, 0, sizeof(signalAction));
    signalAction.sa_handler = signalInterruptHandler;
    sigfillset(&signalAction.sa_mask);
    sigaction(SIGINT, &signalAction, nullptr);
    sigaction(SIGTERM, &signalAction, nullptr);
    sigaction(SIGSTOP, &signalAction, nullptr);

    // Run main app
    handler = std::make_unique<TTTextBoxHandler>(uniqueName, &messageSent, &contactsSwitch);
    while (!handler->stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}