#include "TTContacts.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTDiagnosticsTracer.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <signal.h>
#include <atomic>

// Application
std::unique_ptr<TTContacts> application;
const std::string LOGGER_PREFIX = "Main:";
TTDiagnosticsLogger TTDiagnosticsLogger::mInstance("tteams-contacts");

void signalInterruptHandler(int) {
    if (application) {
        application->stop();
    }
}

int main(int argc, char** argv) {
    DT_INIT(DT_UNIQUE_PATH("tteams-contacts").c_str());
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    DT_META_PROCESS_NAME("tteams-contacts");
    DT_META_THREAD_NAME("main");
    try {
        // Initialize
        DT_BEGIN("main", "initialization");
        // Set signal handling
        struct sigaction signalAction;
        memset(&signalAction, 0, sizeof(signalAction));
        signalAction.sa_handler = signalInterruptHandler;
        sigfillset(&signalAction.sa_mask);
        sigaction(SIGINT, &signalAction, nullptr);
        sigaction(SIGTERM, &signalAction, nullptr);
        sigaction(SIGSTOP, &signalAction, nullptr);
        logger.info("{} Signal handling initialized", LOGGER_PREFIX);
        // Set contacts
        const TTContactsSettings settings(argc, argv);
        const TTUtilsOutputStream outputStream;
        application = std::make_unique<TTContacts>(settings, outputStream);
        logger.info("{} Contacts initialized", LOGGER_PREFIX);
        DT_END("main", "initialization");
        // Run main app
        DT_BEGIN("main", "run");
        try {
            if (!quitHandle.load()) {
                application->run();
            } else {
                logger.warning("{} Application was shut down out of a sudden", LOGGER_PREFIX);
            }
        } catch (const std::exception& exp) {
            logger.info("{} Exception captured: {}", LOGGER_PREFIX, exp.what());
            throw;
        }
        DT_END("main", "run");
    } catch (const std::exception& exp) {
        logger.info("{} Exception captured: {}", LOGGER_PREFIX, exp.what());
    }
    logger.info("{} Successfully flushed all logs", LOGGER_PREFIX);
    DT_FLUSH();
    DT_SHUTDOWN();
    return 0;
}
