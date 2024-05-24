#include "TTChat.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTDiagnosticsTracer.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <signal.h>
#include <atomic>

// Application
std::unique_ptr<TTChat> application;
const std::string LOGGER_PREFIX = "Main:";
TTDiagnosticsLogger TTDiagnosticsLogger::mInstance("tteams-chat");

void signalInterruptHandler(int) {
    if (application) {
        TTDiagnosticsLogger::getInstance().warning("{} Stopping due to caught signal", LOGGER_PREFIX);
        application->stop();
    }
}

int main(int argc, char** argv) {
    DT_INIT(DT_UNIQUE_PATH("tteams-chat").c_str());
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    DT_META_PROCESS_NAME("tteams-chat");
    DT_META_THREAD_NAME("main");
    try {
        // Initialize
        DT_BEGIN("main", "initialization");
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
        DT_END("main", "initialization");
        DT_BEGIN("main", "run");
        application->run();
        DT_END("main", "run");
    } catch (const std::exception& exp) {
        logger.info("{} Exception captured: {}", LOGGER_PREFIX, exp.what());
    }
    logger.info("{} Successfully flushed all logs", LOGGER_PREFIX);
    DT_FLUSH();
    DT_SHUTDOWN();
    return 0;
}