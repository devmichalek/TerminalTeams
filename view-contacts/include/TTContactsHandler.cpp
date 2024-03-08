#include "TTContactsHandler.hpp"
#include <iostream>
#include <chrono>
#include <list>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContactsHandler::TTContactsHandler(std::string sharedName) :
        mSharedName(sharedName), mSharedMessage(nullptr),
        mDataProducedSemaphore(nullptr), mDataConsumedSemaphore(nullptr), mHandlerThread{} {
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

    mThreadForceExit.store(false);
    mHandlerThread = std::thread(&TTContactsHandler::main, this);
    mHandlerThread.detach();
}

TTContactsHandler::~TTContactsHandler() {
    {   // Shutdown thread
        mThreadForceExit.store(true);
        mQueueCondition.notify_one();
        // Wait until thread is closed
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mQueueCondition.wait(lock, [this]() {
            return !mThreadForceExit.load();
        });
    }
    clean();
}

void TTContactsHandler::create(std::string nickname, std::string fullname, std::string decription, std::string ipAddressAndPort) {
    TTContactsMessage message;
    message.status = TTContactsStatus::ACTIVE;
    message.id = mContacts.size();
    message.dataLength = nickname.size();
    memset(&message.data[0], 0, TTCONTACTS_DATA_MAX_LENGTH);
    memcpy(&message.data[0], nickname.c_str(), message.dataLength);
    send(message);
    mContacts.emplace_back(nickname, fullname, decription, ipAddressAndPort);
}

bool TTContactsHandler::send(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::SELECTED_ACTIVE: break;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = TTContactsStatus::PENDING_MSG_INACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: break;
        case TTContactsStatus::ACTIVE:
        case TTContactsStatus::INACTIVE:
        case TTContactsStatus::UNREAD_MSG_ACTIVE:
        case TTContactsStatus::UNREAD_MSG_INACTIVE:
        case TTContactsStatus::PENDING_MSG_INACTIVE:
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    mContacts[id].sentMessages++;
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    send(message);
    return true;
}

bool TTContactsHandler::receive(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_ACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE:
        case TTContactsStatus::PENDING_MSG_INACTIVE:
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE:
        case TTContactsStatus::SELECTED_INACTIVE:
        case TTContactsStatus::INACTIVE:
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    mContacts[id].receivedMessages++;
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    send(message);
    return false;
}

bool TTContactsHandler::activate(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: break;
        case TTContactsStatus::INACTIVE: mContacts[id].status = TTContactsStatus::ACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: break;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_ACTIVE; break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::ACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    send(message);
    return true;
}

bool TTContactsHandler::deactivate(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::INACTIVE; break;
        case TTContactsStatus::INACTIVE: break;
        case TTContactsStatus::SELECTED_ACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_INACTIVE; break;
        case TTContactsStatus::SELECTED_INACTIVE: break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_INACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: break;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    send(message);
    return true;
}

bool TTContactsHandler::select(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = SELECTED_ACTIVE; break;
        case TTContactsStatus::INACTIVE: mContacts[id].status = SELECTED_INACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: break;
        case TTContactsStatus::SELECTED_INACTIVE: break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: mContacts[id].status = SELECTED_ACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: mContacts[id].status = SELECTED_INACTIVE; break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: mContacts[id].status = SELECTED_PENDING_MSG_INACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: break;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    send(message);
    return true;
}

const TTContactsEntry& TTContactsHandler::get(size_t id) {
    return mContacts[id];
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

void TTContactsHandler::main() {
    try {
        bool exit = false;
        do {
            // Fill the list of messages
            std::list<std::unique_ptr<TTContactsMessage>> messages;
            {
                std::unique_lock<std::mutex> lock(mQueueMutex);
                mQueueCondition.wait(lock, [this]() {
                    return !mQueuedMessages.empty() || mThreadForceExit.load();
                });

                if (mThreadForceExit.load()) {
                    exit = true;
                    break; // Soft failure, forced exit
                }

                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
            }

            for (auto &message : messages) {
                if (mThreadForceExit.load()) {
                    exit = true;
                    break; // Soft failure, forced exit
                }
                // Set shared message
                memcpy(mSharedMessage, message.get(), sizeof(TTContactsMessage));
                // Inform other process about produced data
                if (sem_post(mDataProducedSemaphore) == -1) {
                    exit = true;
                    break; // Hard failure
                }
                // Wait for the other process to consume the data
                {
                    timespec ts;
                    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        exit = true;
                        break; // Hard failure
                    }
                    ts.tv_nsec = DATA_CONSUME_TIMEOUT_NS;
                    auto tryCount = DATA_CONSUME_TRY_COUNT;
                    int result = 0;
                    for (auto attempt = DATA_CONSUME_TRY_COUNT; attempt > 0; --attempt) {
                        result = sem_timedwait(mDataConsumedSemaphore, &ts);
                        if (result != -1) {
                            break; // Success
                        } else if (errno == EINTR) {
                            continue; // Soft failure
                        }
                    }
                    if (result == -1) {
                        exit = true;
                        break; // Hard failure
                    }
                }
            }
        } while (!exit);
    } catch (...) {
        // ...
    }

    // Wait for the main thread to call destructor
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mQueueCondition.wait(lock, [this]() {
            return mThreadForceExit.load();
        });
    }

    mThreadForceExit.store(false);
    mQueueCondition.notify_one();
}

int main(int argc, char** argv) {
    std::string dummyString;
    TTContactsMessage dummyMessage;
	TTContactsHandler handler("contacts");
    while (true) {
        std::getline(std::cin, dummyString);
        switch (dummyString[0]) {
            case 'c': handler.create(dummyString.substr(2), "", "", ""); break;
            case 's': handler.send(std::stoi(dummyString.substr(2))); break;
            case 'r': handler.receive(std::stoi(dummyString.substr(2))); break;
            case 'a': handler.activate(std::stoi(dummyString.substr(2))); break;
            case 'd': handler.deactivate(std::stoi(dummyString.substr(2))); break;
            case 'z': handler.select(std::stoi(dummyString.substr(2))); break;
            case 'e': return 0;

        }
    }
	return 0;
}