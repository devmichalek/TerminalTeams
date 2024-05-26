#include "TTContactsHandler.hpp"
#include <iostream>
#include <chrono>
#include <list>
#include <cstring>

TTContactsHandler::TTContactsHandler(const TTContactsSettings& settings) :
        mSharedMem(std::move(settings.getSharedMemory())),
        mForcedQuit{false},
        mHandlerThread{&TTContactsHandler::main, this},
        mHeartbeatThread{&TTContactsHandler::heartbeat, this} {
    LOG_INFO("Constructing...");
    if (!mSharedMem->create()) {
        throw std::runtime_error("TTContactsHandler: Failed to create shared memory!");
    } else {
        LOG_INFO("Successfully created shared memory!");
    }
    mHandlerThread.detach();
    mHeartbeatThread.detach();
    LOG_INFO("Successfully detached threads");
}

TTContactsHandler::~TTContactsHandler() {
    LOG_INFO("Destructing...");
    mForcedQuit.store(true);
    mQueueCondition.notify_one();
    // Wait until main thread is destroyed
    std::scoped_lock<std::mutex> handlerQuitLock(mHandlerQuitMutex);
    // Wait until heartbeat thread is destroyed
    std::scoped_lock<std::mutex> heartbeatQuitLock(mHeartbeatQuitMutex);
}

bool TTContactsHandler::create(std::string nickname, std::string fullname, std::string ipAddressAndPort) {
    TTContactsMessage message;
    message.status = TTContactsStatus::ACTIVE;
    message.id = mContacts.size();
    message.dataLength = nickname.size();
    memset(&message.data[0], 0, TTCONTACTS_DATA_MAX_LENGTH);
    memcpy(&message.data[0], nickname.c_str(), message.dataLength);
    mContacts.emplace_back(nickname, fullname, ipAddressAndPort);
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
                if (!mSharedMem->send(reinterpret_cast<void*>(message.get()))) {
                    exit = true;
                    break; // Hard failure
                }
            }
        } while (!exit);
    } catch (...) {
        // ...
    }
    mSharedMem->destroy();
    mForcedQuit.store(true);
}
