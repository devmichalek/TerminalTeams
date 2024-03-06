#include "TTContacts.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContacts::TTContacts(TTContactsSettings settings) :
		mSharedName(settings.getSharedName()), mSharedMessage(nullptr) {
	const std::string classNamePrefix = "TTContacts: ";
	std::string dataProducedSemName = mSharedName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
	std::string dataConsumedSemName = mSharedName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);

	const int tryCount = 5;
	const int tryIntervalMs = 2000;
	errno = 0;
	for (int i = 0; i < tryCount; --i) {
		if ((mDataProducedSemaphore = sem_open(dataProducedSemName.c_str(), 0)) != SEM_FAILED) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(tryIntervalMs));
	}

	if (mDataProducedSemaphore == SEM_FAILED) {
		throw std::runtime_error(classNamePrefix + "Failed to open data produced semaphore, errno=" + std::to_string(errno));
	}

	errno = 0;
	if ((mDataConsumedSemaphore = sem_open(dataConsumedSemName.c_str(), 0)) == SEM_FAILED) {
		throw std::runtime_error(classNamePrefix + "Failed to open data consumed semaphore, errno=" + std::to_string(errno));
	}

	int fd = shm_open(mSharedName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		throw std::runtime_error(classNamePrefix + "Failed to open shared object, errno=" + std::to_string(errno));
	}

	void* rawPointer = mmap(nullptr, sizeof(TTContactsMessage), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	mSharedMessage = new(rawPointer) TTContactsMessage;
}

TTContacts::~TTContacts() {
	shm_unlink(mSharedName.c_str());
    std::string dataProducedSemName = mSharedName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
    sem_unlink(dataProducedSemName.c_str());
    std::string dataConsumedSemName = mSharedName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);
    sem_unlink(dataConsumedSemName.c_str());
}

void TTContacts::run() {
	while (true) {
		if (sem_wait(mDataProducedSemaphore) == -1) {
			break;
		}

		std::cout << "Received data == " << mSharedMessage->dataLength << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		// ...
		if (sem_post(mDataConsumedSemaphore) == -1) {
			break;
		}
	}
}

int main(int argc, char** argv) {
	TTContactsSettings settings(argc, argv);
	TTContacts contacts(settings);
	contacts.run();
	return 0;
}