#include "TTContactsHandler.hpp"
#include <iostream>
#include <chrono>
#include <list>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContactsHandler::TTContactsHandler(std::string sharedMemoryName,
    TTContactsCallbackDataProduced callbackDataProduced,
    TTContactsCallbackDataConsumed callbackDataConsumed) :
		mCallbackDataProduced(callbackDataProduced),
		mCallbackDataConsumed(callbackDataConsumed),
        mSharedMemoryName(sharedMemoryName),
        mSharedMessage(nullptr),
        mDataProducedSemaphore(nullptr),
        mDataConsumedSemaphore(nullptr),
        mForcedQuit{false},
        mHandlerThread{&TTContactsHandler::main, this},
        mHeartbeatThread{&TTContactsHandler::heartbeat, this} {
	const std::string classNamePrefix = "TTContactsHandler: ";
	errno = 0;
	int fd = shm_open(mSharedMemoryName.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
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
	std::string dataProducedSemName = mSharedMemoryName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
	if ((mDataProducedSemaphore = sem_open(dataProducedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
		throw std::runtime_error(classNamePrefix + "Failed to create data produced semaphore, errno=" + std::to_string(errno));
	}

	errno = 0;
	std::string dataConsumedSemName = mSharedMemoryName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);
	if ((mDataConsumedSemaphore = sem_open(dataConsumedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
		throw std::runtime_error(classNamePrefix + "Failed to create data consumed semaphore, errno=" + std::to_string(errno));
	}

    mHandlerThread.detach();
    mHeartbeatThread.detach();
}

TTContactsHandler::~TTContactsHandler() {
    mForcedQuit.store(true);
    mQueueCondition.notify_one();
    // Wait until main thread is destroyed
    std::scoped_lock<std::mutex> handlerQuitLock(mHandlerQuitMutex);
    // Wait until heartbeat thread is destroyed
    std::scoped_lock<std::mutex> heartbeatQuitLock(mHeartbeatQuitMutex);
}

bool TTContactsHandler::create(std::string nickname, std::string fullname, std::string decription, std::string ipAddressAndPort) {
    TTContactsMessage message;
    message.status = TTContactsStatus::ACTIVE;
    message.id = mContacts.size();
    message.dataLength = nickname.size();
    memset(&message.data[0], 0, TTCONTACTS_DATA_MAX_LENGTH);
    memcpy(&message.data[0], nickname.c_str(), message.dataLength);
    mContacts.emplace_back(nickname, fullname, decription, ipAddressAndPort);
    return send(message);
}

bool TTContactsHandler::send(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::SELECTED_ACTIVE: return false;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = TTContactsStatus::PENDING_MSG_INACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: return false;
        case TTContactsStatus::ACTIVE: return false;
        case TTContactsStatus::INACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: return false;
        case TTContactsStatus::PENDING_MSG_INACTIVE: return false;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    mContacts[id].sentMessages++;
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::receive(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_ACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: return false;
        case TTContactsStatus::PENDING_MSG_INACTIVE: return false;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: return false;
        case TTContactsStatus::SELECTED_INACTIVE: return false;
        case TTContactsStatus::INACTIVE: return false;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    mContacts[id].receivedMessages++;
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::activate(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: return false;
        case TTContactsStatus::INACTIVE: mContacts[id].status = TTContactsStatus::ACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: return false;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_ACTIVE; break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::ACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::deactivate(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::INACTIVE; break;
        case TTContactsStatus::INACTIVE: return false;
        case TTContactsStatus::SELECTED_ACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_INACTIVE; break;
        case TTContactsStatus::SELECTED_INACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_INACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: return false;
        case TTContactsStatus::PENDING_MSG_INACTIVE: return false;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: return false;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::select(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = SELECTED_ACTIVE; break;
        case TTContactsStatus::INACTIVE: mContacts[id].status = SELECTED_INACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: return false;
        case TTContactsStatus::SELECTED_INACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: mContacts[id].status = SELECTED_ACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: mContacts[id].status = SELECTED_INACTIVE; break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: mContacts[id].status = SELECTED_PENDING_MSG_INACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: return false;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::unselect(size_t id) {
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: return false;
        case TTContactsStatus::INACTIVE: return false;
        case TTContactsStatus::SELECTED_ACTIVE: mContacts[id].status = ACTIVE; break;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = INACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: return false;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: return false;
        case TTContactsStatus::PENDING_MSG_INACTIVE: return false;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: mContacts[id].status = PENDING_MSG_INACTIVE; break;
        default:
            throw std::runtime_error("Failed to change contact status, internal error");
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

const TTContactsEntry& TTContactsHandler::get(size_t id) const {
    return mContacts[id];
}

bool TTContactsHandler::send(const TTContactsMessage& message) {
    if (mForcedQuit.load()) {
        return false;
    }
    {
        std::scoped_lock<std::mutex> lock(mQueueMutex);
        mQueuedMessages.push(std::make_unique<TTContactsMessage>(message));
    }
	mQueueCondition.notify_one();
    return true;
}

void TTContactsHandler::heartbeat() {
    std::scoped_lock<std::mutex> heartbeatQuitLock(mHeartbeatQuitMutex);
    try {
        while (!mForcedQuit.load()) {
            TTContactsMessage message;
            message.status = TTContactsStatus::HEARTBEAT;
            if (!send(message)) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(TTCONTACTS_HEARTBEAT_TIMEOUT_MS));
        }
    } catch (...) {
        // ...
    }
    mForcedQuit.store(true);
}

void TTContactsHandler::main() {
    std::scoped_lock<std::mutex> handlerQuitLock(mHandlerQuitMutex);
    try {
        bool exit = false;
        do {
            // Fill the list of messages
            std::list<std::unique_ptr<TTContactsMessage>> messages;
            {
                std::unique_lock<std::mutex> lock(mQueueMutex);
                mQueueCondition.wait(lock, [this]() {
                    return !mQueuedMessages.empty() || mForcedQuit.load();
                });

                if (mForcedQuit.load()) {
                    exit = true;
                    break; // Forced exit
                }

                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
            }

            for (auto &message : messages) {
                if (mForcedQuit.load()) {
                    exit = true;
                    break; // Forced exit
                }
                // Set shared message
                memcpy(mSharedMessage, message.get(), sizeof(TTContactsMessage));
                // Inform other process about produced data
                if (sem_post(mDataProducedSemaphore) == -1) {
                    exit = true;
                    break; // Hard failure
                } else if (mCallbackDataProduced) {
                    mCallbackDataProduced();
                }

                // Wait for the other process to consume the data
                int result = 0;
                for (auto attempt = TTCONTACTS_DATA_CONSUME_TRY_COUNT; attempt >= 0; --attempt) {
                    struct timespec ts;
                    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        result = -1;
                        break; // Hard failure
                    }
                    ts.tv_sec += TTCONTACTS_DATA_CONSUME_TIMEOUT_S;
                    result = sem_timedwait(mDataConsumedSemaphore, &ts);
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
                    exit = true;
                    break; // Hard failure
                }
            }
        } while (!exit);
    } catch (...) {
        // ...
    }
    shm_unlink(mSharedMemoryName.c_str());
    std::string dataProducedSemName = mSharedMemoryName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
    sem_unlink(dataProducedSemName.c_str());
    std::string dataConsumedSemName = mSharedMemoryName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);
    sem_unlink(dataConsumedSemName.c_str());
    mForcedQuit.store(true);
}
