#include "TTTextBox.hpp"
#include <signal.h>
#include <memory>

std::unique_ptr<TTTextBox> application;

void signalInterruptHandler(int) {
    if (application) {
        application->stop();
    }
}

int main(int argc, char** argv) {
    // Signal handling
    struct sigaction signalAction;
    memset(&signalAction, 0, sizeof(signalAction));
    signalAction.sa_handler = signalInterruptHandler;
    sigfillset(&signalAction.sa_mask);
    sigaction(SIGINT, &signalAction, nullptr);
    sigaction(SIGTERM, &signalAction, nullptr);
    sigaction(SIGSTOP, &signalAction, nullptr);

    // Run application
    TTTextBoxSettings settings(argc, argv);
    application = std::make_unique<TTTextBox>(settings);
    application->run();
    return 0;
}