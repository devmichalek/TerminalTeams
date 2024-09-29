#include "TTChat.hpp"
#include "TTUtilsSignals.hpp"
#include "TTConfig.hpp"

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
    LOG_INFO("{}", VERSION_STRING);
    try {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.setup(signalInterruptHandler, { SIGINT, SIGTERM, SIGSTOP });
        // Run main app
        TTChatSettings settings(argc, argv);
        TTUtilsOutputStream outputStream;
        application = std::make_unique<TTChat>(settings, outputStream);
        application->run();
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    application.reset();
    LOG_INFO("Successfully flushed all logs");
    return 0;
}