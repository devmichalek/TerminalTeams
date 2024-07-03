#include "TTChat.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <signal.h>

// Application
std::unique_ptr<TTChat> application;
LOG_DECLARE("tteams-chat");

void signalInterruptHandler(int) {
    if (application) {
        LOG_WARNING("Stopping due to caught signal");
        application->stop();
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
        // Run main app
        TTChatSettings settings(argc, argv);
        const TTUtilsOutputStream outputStream;
        application = std::make_unique<TTChat>(settings, outputStream);
        application->run();
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    application.reset();
    LOG_INFO("Successfully flushed all logs");
    return 0;
}