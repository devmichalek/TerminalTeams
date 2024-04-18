#include "TTContacts.hpp"
#include "TTContactsOutputStream.hpp"
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
    // Signal handling
    struct sigaction signalAction;
    memset(&signalAction, 0, sizeof(signalAction));
    signalAction.sa_handler = signalInterruptHandler;
    sigfillset(&signalAction.sa_mask);
    sigaction(SIGINT, &signalAction, nullptr);
    sigaction(SIGTERM, &signalAction, nullptr);
    sigaction(SIGSTOP, &signalAction, nullptr);

    // Run main app
    const TTContactsSettings settings(argc, argv);
    const TTContactsOutputStream outputStream;
    TTContacts contacts(settings, &quit, outputStream);
    if (!quitHandle.load()) {
        contacts.run();
    }
    return 0;
}
