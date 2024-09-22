#include "TTEngine.hpp"
#include "TTDiagnosticsLogger.hpp"
#include "TTConfig.hpp"
#include <signal.h>

// Application
std::unique_ptr<TTEngine> application;
LOG_DECLARE("tteams-engine");

void signalInterruptHandler(int) {
    if (application) {
        LOG_WARNING("Stopping due to caught signal");
        application->stop();
    }
}

int main(int argc, char** argv) {
    LOG_INFO("{}", VERSION_STRING);
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
        TTEngineSettings settings(argc, argv);
        application = std::make_unique<TTEngine>(settings);
        // Run main app
        try {
            if (!application->stopped()) {
                application->run();
            } else {
                LOG_WARNING("Application was shut down out of a sudden");
            }
        } catch (const std::exception& exp) {
            LOG_ERROR("Exception captured: {}", exp.what());
            throw;
        }
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    application.reset();
    LOG_INFO("Successfully flushed all logs");
    return 0;
}

