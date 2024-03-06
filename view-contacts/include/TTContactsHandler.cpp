#include "TTContactsHandler.hpp"
#include <iostream>
#include <chrono>
#include <list>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContactsHandler::TTContactsHandler(std::string sharedName) :
        mSharedName(sharedName), mSharedMessage(nullptr), mDataProducedSemaphore(nullptr), mDataConsumedSemaphore(nullptr) {
    clean();
    
	const std::string classNamePrefix = "TTContactsHandler: ";
	errno = 0;
	int fd = shm_open(mSharedName.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		throw std::runtime_error(classNamePrefix + "Failed to create shared object, errno=" + std::to_string(errno));
	}

	errno = 0;
	if (ftruncate(fd, sizeof(TTContactsMessage)) == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to truncate shared object, errno=" + std::to_string(errno));
	}

	void* rawPointer = mmap(nullptr, sizeof(TTContactsMessage), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	mSharedMessage = new(rawPointer) TTContactsMessage;

	errno = 0;
	std::string dataProducedSemName = mSharedName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
	if ((mDataProducedSemaphore = sem_open(dataProducedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
		throw std::runtime_error(classNamePrefix + "Failed to create data produced semaphore, errno=" + std::to_string(errno));
	}

	errno = 0;
	std::string dataConsumedSemName = mSharedName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);
	if ((mDataConsumedSemaphore = sem_open(dataConsumedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
		throw std::runtime_error(classNamePrefix + "Failed to create data consumed semaphore, errno=" + std::to_string(errno));
	}

    std::thread handlerThread(&TTContactsHandler::run, this);
    handlerThread.detach();
}

TTContactsHandler::~TTContactsHandler() {
    clean();
}

void TTContactsHandler::send(const TTContactsMessage& message) {
    {
        std::scoped_lock<std::mutex> lock(mQueueMutex);
        mQueuedMessages.push(std::make_unique<TTContactsMessage>(message));
    }
	mQueueCondition.notify_one();
}

void TTContactsHandler::clean() {
    shm_unlink(mSharedName.c_str());
    std::string dataProducedSemName = mSharedName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
    sem_unlink(dataProducedSemName.c_str());
    std::string dataConsumedSemName = mSharedName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);
    sem_unlink(dataConsumedSemName.c_str());
}

void TTContactsHandler::run() {
    std::list<std::unique_ptr<TTContactsMessage>> messages;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mQueueCondition.wait(lock, [this]() { return !mQueuedMessages.empty(); }); 
            while (!mQueuedMessages.empty()) {
                messages.push_back(std::move(mQueuedMessages.front()));
                mQueuedMessages.pop();
            }
            std::cout << mQueuedMessages.size() << "\n";
            std::cout << messages.size() << "\n";
        }

        for (auto &message : messages) {
            TTContactsMessage tmpMessage;
            memcpy(mSharedMessage, &tmpMessage, sizeof(TTContactsMessage));

            if (sem_post(mDataProducedSemaphore) == -1) {
                break;
            }

            std::cout << "Data produced!\n";
            
            if (sem_wait(mDataConsumedSemaphore) == -1) {
                break;
            }

            std::cout << "Data consumed!\n";
        }
        messages.clear();
	}
}


// int main(int argc, char** argv) {
//     std::string dummyString;
//     TTContactsMessage dummyMessage;
// 	TTContactsHandler handler("contacts");
//     while (true) {
//         std::getline(std::cin, dummyString);
//         if (dummyString.length() != 0) {
//             break;
//         }
//         handler.send(dummyMessage);
//     }
// 	return 0;
// }