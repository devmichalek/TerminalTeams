#include "TTTextBox.hpp"
#include "TTUtilsSignals.hpp"
#include "TTConfig.hpp"

std::unique_ptr<TTTextBox> application;
LOG_DECLARE("tteams-textbox");

void signalInterruptHandler(int) {
    if (application) {
        LOG_WARNING("Stopping due to caught signal");
        application->stop();
    }
}

int main(int argc, char** argv) {
    LOG_INFO("{}", VERSION_STRING);
    try {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.setup(signalInterruptHandler, { SIGINT, SIGTERM, SIGSTOP });
        // Run application
        TTTextBoxSettings settings(argc, argv);
        TTUtilsOutputStream outputStream;
        TTUtilsInputStream inputStream;
        application = std::make_unique<TTTextBox>(settings, outputStream, inputStream);
        LOG_INFO("TextBox initialized");
        try {
            if (!application->isStopped()) {
                application->wait();
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