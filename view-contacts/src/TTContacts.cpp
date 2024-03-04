#include "TTContacts.hpp"
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContacts::TTContacts(TTContactsSettings settings) : mSharedMessage(nullptr) {
	int fd = -1;
	int tryCount = 5;
	const int tryIntervalMs = 2000;
	while (tryCount--) {
		fd = shm_open(settings.getSharedName().c_str(), O_RDONLY, S_IRUSR);
		std::this_thread::sleep_for(std::chrono::milliseconds(tryIntervalMs));
	}

	const std::string classNamePrefix = "TTContacts: ";
	if (fd < 0) {
		throw std::runtime_error(classNamePrefix + "Failed to open shared object, errno=" + std::to_string(fd));
	}

	// todo on engine side
	// if (ftruncate(fd, sizeof(TTContactsSharedMessage) < 0)) {
	// 	throw std::runtime_error(classNamePrefix + "Failed to truncate shared object, errno=" + std::to_string(fd));
	// }

	mSharedMessage = mmap(nullptr, sizeof(TTContactsSharedMessage), PROT_READ, MAP_SHARED, fd, 0);
}

void TTContacts::run() {
	
}

int main(int argc, char** argv) {
	TTContactsSettings settings(argc, argv);
	TTContacts contacts(settings);
	contacts.run();
	return 0;
}