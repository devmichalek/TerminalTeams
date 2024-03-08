#include "TTContacts.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContacts::TTContacts(TTContactsSettings settings) :
		mSharedName(settings.getSharedName()), mTerminalWidth(settings.getTerminalWidth()),
		mTerminalHeight(settings.getTerminalHeight()), mSharedMessage(nullptr) {
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
}

void TTContacts::run() {
	std::vector<std::string> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
	while (true) {
		if (sem_wait(mDataProducedSemaphore) == -1) {
			break;
		}

		TTContactsMessage newMessage;
		memcpy(&newMessage, mSharedMessage, sizeof(newMessage));

		if (sem_post(mDataConsumedSemaphore) == -1) {
			break;
		}

		if (newMessage.status == TTContactsStatus::ACTIVE) {
			if (newMessage.id >= mContacts.size()) {
				std::string nickname(newMessage.data, newMessage.data + newMessage.dataLength);
				auto newContact = std::make_tuple(newMessage.id, nickname, newMessage.status);
				mContacts.push_back(newContact);
			}
		} else {
			auto& contact = mContacts[newMessage.id];
				std::get<2>(contact) = newMessage.status;
		}

		system("clear");

		for (auto &contact : mContacts) {
			std::cout << "#" << std::get<0>(contact);
			std::cout << " " << std::get<1>(contact);
			std::cout << " " << statuses[std::get<2>(contact)];
			std::cout << std::endl;
		}
	}
}

int main(int argc, char** argv) {
	TTContactsSettings settings(argc, argv);
	TTContacts contacts(settings);
	contacts.run();
	return 0;
}