#include "TTContacts.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContacts::TTContacts(TTContactsSettings settings,
	TTContactsCallbackQuit callbackQuit,
	TTContactsCallbackDataProduced callbackDataProduced,
	TTContactsCallbackDataConsumed callbackDataConsumed) :
		mCallbackQuit(callbackQuit),
		mCallbackDataProduced(callbackDataProduced),
		mCallbackDataConsumed(callbackDataConsumed),
		mSharedName(settings.getSharedName()),
		mSharedMessage(nullptr),
		mDataProducedSemaphore(nullptr),
		mDataConsumedSemaphore(nullptr),
		mTerminalWidth(settings.getTerminalWidth()),
		mTerminalHeight(settings.getTerminalHeight()) {
	const std::string classNamePrefix = "TTContacts: ";
	std::string dataProducedSemName = mSharedName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
	std::string dataConsumedSemName = mSharedName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);

	errno = 0;
	for (auto attempt = TTCONTACTS_SEMAPHORES_READY_TRY_COUNT; attempt > 0; --attempt) {
		if (mCallbackQuit && mCallbackQuit()) {
			return; // Forced exit
		}
		if ((mDataProducedSemaphore = sem_open(dataProducedSemName.c_str(), 0)) != SEM_FAILED) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(TTCONTACTS_SEMAPHORES_READY_TIMEOUT_MS));
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

void TTContacts::run() {
	if (!mSharedMessage) {
		return;
	}

	std::vector<std::string> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
	while (true) {
		// Wait for the other process to produce the data
		{
			timespec ts;
			if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
				break; // Hard failure
			}
			ts.tv_sec = TTCONTACTS_DATA_PRODUCE_TIMEOUT_S;
			int result = 0;
			for (auto attempt = TTCONTACTS_DATA_PRODUCE_TRY_COUNT; attempt > 0; --attempt) {
				if (mCallbackQuit && mCallbackQuit()) {
					break; // Forced exit
				}
				result = sem_timedwait(mDataProducedSemaphore, &ts);
				if (result != -1) {
					if (mCallbackDataConsumed) {
						mCallbackDataConsumed();
					}
					break; // Success
				} else if (errno == EINTR) {
					continue; // Soft failure
				}
			}
			if (result == -1) {
				break; // Hard failure
			}
		}

		TTContactsMessage newMessage;
		memcpy(&newMessage, mSharedMessage, sizeof(newMessage));

		if (sem_post(mDataConsumedSemaphore) == -1) {
			break;
		} else if (mCallbackDataProduced) {
			mCallbackDataProduced();
		}

		if (newMessage.status == TTContactsStatus::HEARTBEAT) {
			// Nothing to be done
			continue;
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
