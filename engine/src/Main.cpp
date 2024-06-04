#include "TTEngine.hpp"
#include "TTDiagnosticsTracer.hpp"
#include "TTDiagnosticsLogger.hpp"
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
    DT_INIT(DT_UNIQUE_PATH("tteams-engine").c_str());
    DT_META_PROCESS_NAME("tteams-engine");
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
        TTEngineSettings settings(argc, argv);
        application = std::make_unique<TTEngine>(settings);
        DT_END("main", "initialization");
        // Run main app
        DT_BEGIN("main", "run");
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
        DT_END("main", "run");
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    application.reset();
    LOG_INFO("Successfully flushed all logs");
    DT_FLUSH();
    DT_SHUTDOWN();
    return 0;
}

