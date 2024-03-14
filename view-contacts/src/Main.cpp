#include "TTContacts.hpp"
#include <signal.h>
#include <atomic>

std::atomic<bool> quitHandle{false};
bool quit() {
	return quitHandle.load();
}

std::atomic<size_t> producedCounter{0};
void produced() {
	producedCounter++;
}

std::atomic<size_t> consumedCounter{0};
void consumed() {
	consumedCounter++;
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

	// Run main app
	TTContactsSettings settings(argc, argv);
	TTContacts contacts(settings, &quit, &produced, &consumed);
	if (!quitHandle.load()) {
		contacts.run();
	}
	return 0;
}
