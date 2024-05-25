#include "TTTextBox.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <signal.h>
#include <memory>

std::unique_ptr<TTTextBox> application;
const std::string LOGGER_PREFIX = "Main:";
TTDiagnosticsLogger TTDiagnosticsLogger::mInstance("tteams-textbox");

void signalInterruptHandler(int) {
    if (application) {
        TTDiagnosticsLogger::getInstance().warning("{} Stopping due to caught signal", LOGGER_PREFIX);
        application->stop();
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
        // Run application
        TTTextBoxSettings settings(argc, argv);
        const TTUtilsOutputStream outputStream;
        application = std::make_unique<TTTextBox>(settings, outputStream);
        logger.info("{} Textbox initialized", LOGGER_PREFIX);
        try {
            if (!application->stopped()) {
                application->run();
            } else {
                logger.warning("{} Application was shut down out of a sudden", LOGGER_PREFIX);
            }
        } catch (const std::exception& exp) {
            logger.info("{} Exception captured: {}", LOGGER_PREFIX, exp.what());
            throw;
        }
    } catch (const std::exception& exp) {
        logger.info("{} Exception captured: {}", LOGGER_PREFIX, exp.what());
    }
    application.reset();
    logger.info("{} Successfully flushed all logs", LOGGER_PREFIX);
    return 0;
}