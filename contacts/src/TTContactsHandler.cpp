#include "TTContactsHandler.hpp"
#include <iostream>
#include <chrono>
#include <list>
#include <cstring>

TTContactsHandler::TTContactsHandler(const TTContactsSettings& settings) :
        mSharedMem(std::move(settings.getSharedMemory())),
        mStopped{false},
        mHandlerThread{},
        mHeartbeatThread{},
        mCurrentContact(std::numeric_limits<size_t>::max()),
        mPreviousContact(std::numeric_limits<size_t>::max()) {
    LOG_INFO("Constructing...");
    if (!mSharedMem->create()) {
        throw std::runtime_error("TTContactsHandler: Failed to create shared memory!");
    }
    if (!establish()) {
        throw std::runtime_error("TTContactsHandler: Failed to establish connection!");
    }
    mHandlerThread = std::thread{&TTContactsHandler::main, this};
    mHeartbeatThread = std::thread{&TTContactsHandler::heartbeat, this};
    mHandlerThread.detach();
    mHeartbeatThread.detach();
    LOG_INFO("Successfully constructed!");
}

TTContactsHandler::~TTContactsHandler() {
    LOG_INFO("Destructing...");
    stop();
    mQueueCondition.notify_one();
    // Wait until main thread is destroyed
    std::scoped_lock handlerQuitLock(mHandlerQuitMutex);
    // Wait until heartbeat thread is destroyed
    std::scoped_lock heartbeatQuitLock(mHeartbeatQuitMutex);
    LOG_INFO("Successfully destructed!");
}

bool TTContactsHandler::create(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) {
    LOG_INFO("Called create nickname={}, identity={}, ipAddressAndPort={}", nickname, identity, ipAddressAndPort);
    std::scoped_lock contactsLock(mContactsMutex);
    TTContactsMessage message;
    message.status = TTContactsStatus::ACTIVE;
    message.id = mContacts.size();
    message.dataLength = nickname.size();
    memset(&message.data[0], 0, TTCONTACTS_DATA_MAX_LENGTH);
    memcpy(&message.data[0], nickname.c_str(), message.dataLength);
    mContacts.emplace_back(nickname, identity, ipAddressAndPort);
    mIdentityMap[identity] = message.id;
    return send(message);
}

bool TTContactsHandler::send(size_t id) {
    LOG_INFO("Called send ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::SELECTED_ACTIVE: return true;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = TTContactsStatus::PENDING_MSG_INACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: return true;
        case TTContactsStatus::ACTIVE:
        case TTContactsStatus::INACTIVE:
        case TTContactsStatus::UNREAD_MSG_ACTIVE:
        case TTContactsStatus::UNREAD_MSG_INACTIVE:
        case TTContactsStatus::PENDING_MSG_INACTIVE:
        default:
            LOG_ERROR("Failed to change contact status from {} on send", size_t(mContacts[id].status));
            return false;
    }

    mContacts[id].sentMessages++;
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::receive(size_t id) {
    LOG_INFO("Called receive ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_ACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: return true;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: return true;
        case TTContactsStatus::UNREAD_MSG_INACTIVE:
        case TTContactsStatus::PENDING_MSG_INACTIVE:
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE:
        case TTContactsStatus::SELECTED_INACTIVE:
        case TTContactsStatus::INACTIVE:
        default:
            LOG_ERROR("Failed to change contact status from {} on receive", size_t(mContacts[id].status));
            return false;
    }

    mContacts[id].receivedMessages++;
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::activate(size_t id) {
    LOG_INFO("Called activate ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: return true;
        case TTContactsStatus::INACTIVE: mContacts[id].status = TTContactsStatus::ACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE: return true;
        case TTContactsStatus::SELECTED_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: return true;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_ACTIVE; break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::ACTIVE; break;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        default:
            LOG_ERROR("Failed to change contact status from {} on activate", size_t(mContacts[id].status));
            return false;
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::deactivate(size_t id) {
    LOG_INFO("Called deactivate ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) {
        return false;
    }

    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::INACTIVE; break;
        case TTContactsStatus::INACTIVE: return true;
        case TTContactsStatus::SELECTED_ACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_INACTIVE; break;
        case TTContactsStatus::SELECTED_INACTIVE: return true;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: mContacts[id].status = TTContactsStatus::UNREAD_MSG_INACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: return true;
        case TTContactsStatus::PENDING_MSG_INACTIVE: return true;
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: return true;
        default:
            LOG_ERROR("Failed to change contact status from {} on deactivate", size_t(mContacts[id].status));
            return false;
    }

    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

bool TTContactsHandler::select(size_t id) {
    LOG_INFO("Called select ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) {
        return false;
    }

    // Unselect
    mPreviousContact = mCurrentContact;
    if (mPreviousContact != std::numeric_limits<size_t>::max()) {
        switch (mContacts[mPreviousContact].status) {
            case TTContactsStatus::SELECTED_ACTIVE: mContacts[mPreviousContact].status = TTContactsStatus::ACTIVE; break;
            case TTContactsStatus::SELECTED_INACTIVE: mContacts[mPreviousContact].status = TTContactsStatus::INACTIVE; break;
            case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE: mContacts[mPreviousContact].status = TTContactsStatus::PENDING_MSG_INACTIVE; break;
            case TTContactsStatus::ACTIVE:
            case TTContactsStatus::INACTIVE:
            case TTContactsStatus::UNREAD_MSG_ACTIVE:
            case TTContactsStatus::UNREAD_MSG_INACTIVE:
            case TTContactsStatus::PENDING_MSG_INACTIVE:
            default:
                LOG_ERROR("Failed to change contact status from {} on unselect", size_t(mContacts[mPreviousContact].status));
                return false;
        }
        TTContactsMessage message;
        message.status = mContacts[mPreviousContact].status;
        message.id = mPreviousContact;
        if (!send(message)) {
            return false;
        }
    }
    
    // Select
    mCurrentContact = id;
    switch (mContacts[id].status) {
        case TTContactsStatus::ACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        case TTContactsStatus::INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_INACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_ACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_ACTIVE; break;
        case TTContactsStatus::UNREAD_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_INACTIVE; break;
        case TTContactsStatus::PENDING_MSG_INACTIVE: mContacts[id].status = TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE; break;
        case TTContactsStatus::SELECTED_ACTIVE:
        case TTContactsStatus::SELECTED_INACTIVE:
        case TTContactsStatus::SELECTED_PENDING_MSG_INACTIVE:
        default:
            LOG_ERROR("Failed to change contact status from {} on select", size_t(mContacts[id].status));
            return false;
    }
    TTContactsMessage message;
    message.status = mContacts[id].status;
    message.id = id;
    return send(message);
}

std::optional<TTContactsEntry> TTContactsHandler::get(size_t id) const {
    LOG_INFO("Called get ID={}", id);
    std::shared_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) {
        return std::nullopt;
    }
    return {mContacts[id]};
}

std::optional<size_t> TTContactsHandler::get(std::string id) const {
    LOG_INFO("Called get ID={}", id);
    std::shared_lock contactsLock(mContactsMutex);
    decltype(auto) result = mIdentityMap.find(id);
    if (result == mIdentityMap.end()) {
        return std::nullopt;
    }
    return {result->second};
}

size_t TTContactsHandler::current() const {
    std::shared_lock contactsLock(mContactsMutex);
    LOG_INFO("Called current={}", mCurrentContact);
    return mCurrentContact;
}

size_t TTContactsHandler::size() const {
    std::shared_lock contactsLock(mContactsMutex);
    return mContacts.size();
}

void TTContactsHandler::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTContactsHandler::stopped() const {
    return mStopped.load();
}

bool TTContactsHandler::send(const TTContactsMessage& message) {
    if (stopped()) {
        return false;
    }
    {
        std::scoped_lock lock(mQueueMutex);
        mQueuedMessages.push(std::make_unique<TTContactsMessage>(message));
    }
    mQueueCondition.notify_one();
    return true;
}

void TTContactsHandler::heartbeat() {
    LOG_INFO("Started heartbeat loop");
    std::scoped_lock heartbeatQuitLock(mHeartbeatQuitMutex);
    try {
        while (!stopped()) {
            TTContactsMessage message;
            message.status = TTContactsStatus::HEARTBEAT;
            if (!send(message)) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(TTCONTACTS_HEARTBEAT_TIMEOUT_MS));
        }
    } catch (...) {
        LOG_ERROR("Caught unknown exception at heartbeat loop!");
    }
    stop();
    LOG_INFO("Completed heartbeat loop");
}

void TTContactsHandler::main() {
    LOG_INFO("Started main loop");
    std::scoped_lock handlerQuitLock(mHandlerQuitMutex);
    try {
        bool exit = false;
        do {
            // Fill the list of messages
            std::list<std::unique_ptr<TTContactsMessage>> messages;
            {
                std::unique_lock<std::mutex> lock(mQueueMutex);
                mQueueCondition.wait(lock, [this]() {
                    return !mQueuedMessages.empty() || stopped();
                });

                if (stopped()) {
                    exit = true;
                    TTContactsMessage message;
                    message.status = TTContactsStatus::GOODBYE;
                    mSharedMem->send(reinterpret_cast<void*>(&message));
                    break; // Forced exit
                }

                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
            }

            for (auto &message : messages) {
                if (stopped()) {
                    exit = true;
                    TTContactsMessage message;
                    message.status = TTContactsStatus::GOODBYE;
                    mSharedMem->send(reinterpret_cast<void*>(&message));
                    break; // Forced exit
                }
                if (!mSharedMem->send(reinterpret_cast<void*>(message.get()))) {
                    exit = true;
                    break; // Hard failure
                }
            }
        } while (!exit);
    } catch (...) {
        LOG_ERROR("Caught unknown exception at main loop!");
    }
    mSharedMem->destroy();
    stop();
    LOG_INFO("Completed main loop");
}

bool TTContactsHandler::establish() {
    LOG_INFO("Establishing connection...");
    TTContactsMessage message;
    message.status = TTContactsStatus::HEARTBEAT;
    return mSharedMem->send(reinterpret_cast<void*>(&message), 5);
}
