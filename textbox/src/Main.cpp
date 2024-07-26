#include "TTTextBox.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <signal.h>
#include <memory>

std::unique_ptr<TTTextBox> application;
LOG_DECLARE("tteams-textbox");

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
        LOG_INFO("Signal handling initialized");
        // Run application
        TTTextBoxSettings settings(argc, argv);
        TTUtilsOutputStream outputStream;
        TTUtilsInputStream inputStream;
        application = std::make_unique<TTTextBox>(settings, outputStream, inputStream);
        LOG_INFO("Textbox initialized");
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