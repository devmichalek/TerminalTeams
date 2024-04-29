#include "TTContacts.hpp"
#include "TTContactsOutputStream.hpp"
#include "TTDiagnosticsTracer.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <signal.h>
#include <atomic>

std::atomic<bool> quitHandle{false};
bool quit() {
    return quitHandle.load();
}

void signalInterruptHandler(int) {
    quitHandle.store(true);
}

int main(int argc, char** argv) {
    DT_INIT(DT_UNIQUE_PATH("tteams-contacts").c_str());
    auto logger = TTDiagnosticsLogger("tteams-contacts");
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
        logger.info("Signal handling initialized");
        // Set contacts
        const TTContactsSettings settings(argc, argv);
        const TTContactsOutputStream outputStream;
        TTContacts contacts(settings, &quit, outputStream);
        logger.info("Contacts initialized");
        DT_END("main", "initialization");

        // Run main app
        DT_BEGIN("main", "run");
        try {
            if (!quitHandle.load()) {
                contacts.run();
            } else {
                logger.warning("Application was shut down out of a sudden");
            }
        } catch (const std::exception& exp) {
            logger.info("Exception captured: {}", exp.what());
            throw;
        }
        DT_END("main", "run");
    } catch (const std::exception& exp) {
        logger.info("Exception captured: {}", exp.what());
    }
    DT_FLUSH();
    DT_SHUTDOWN();
    return 0;
}
