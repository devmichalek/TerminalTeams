#include "TTContactsHandler.hpp"
#include <iostream>
#include <chrono>
#include <list>
#include <cstring>

TTContactsHandler::TTContactsHandler(const TTContactsSettings& settings) :
        mSharedMem(std::move(settings.getSharedMemory())),
        mHandlerThread{},
        mHeartbeatThread{},
        mCurrentContact(std::nullopt),
        mPreviousContact(std::nullopt) {
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
    message.setStatus(TTContactsStatus::STATE);
    message.setState(TTContactsState::ACTIVE);
    message.setIdentity(mContacts.size());
    message.setNickname(nickname);
    mContacts.emplace_back(nickname, identity, ipAddressAndPort);
    mIdentityMap[identity] = message.getIdentity();
    return send(message);
}

bool TTContactsHandler::send(size_t id) {
    LOG_INFO("Called send ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) [[unlikely]] {
        return false;
    }

    const auto previousState = mContacts[id].state;
    switch (mContacts[id].state) {
        case TTContactsState::SELECTED_ACTIVE: break;
        case TTContactsState::SELECTED_INACTIVE: mContacts[id].state = TTContactsState::SELECTED_PENDING_MSG_INACTIVE; break;
        case TTContactsState::SELECTED_PENDING_MSG_INACTIVE: break;
        case TTContactsState::ACTIVE:
        case TTContactsState::INACTIVE:
        case TTContactsState::UNREAD_MSG_ACTIVE:
        case TTContactsState::UNREAD_MSG_INACTIVE:
        case TTContactsState::PENDING_MSG_INACTIVE:
        default:
            LOG_ERROR("Failed to change contact state from {} on send", size_t(mContacts[id].state));
            return false;
    }

    mContacts[id].sentMessages++;
    if (previousState != mContacts[id].state) [[likely]] {
        TTContactsMessage message;
        message.setStatus(TTContactsStatus::STATE);
        message.setState(mContacts[id].state);
        message.setIdentity(id);
        message.setNickname(mContacts[id].nickname);
        return send(message);
    }
    return true;
}

bool TTContactsHandler::receive(size_t id) {
    LOG_INFO("Called receive ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) [[unlikely]] {
        return false;
    }

    const auto previousState = mContacts[id].state;
    switch (mContacts[id].state) {
        case TTContactsState::ACTIVE: mContacts[id].state = TTContactsState::UNREAD_MSG_ACTIVE; break;
        case TTContactsState::SELECTED_ACTIVE: break;
        case TTContactsState::UNREAD_MSG_ACTIVE: break;
        case TTContactsState::UNREAD_MSG_INACTIVE:
        case TTContactsState::PENDING_MSG_INACTIVE:
        case TTContactsState::SELECTED_PENDING_MSG_INACTIVE:
        case TTContactsState::SELECTED_INACTIVE:
        case TTContactsState::INACTIVE:
        default:
            LOG_ERROR("Failed to change contact state from {} on receive", size_t(mContacts[id].state));
            return false;
    }

    mContacts[id].receivedMessages++;
    if (previousState != mContacts[id].state) [[likely]] {
        TTContactsMessage message;
        message.setStatus(TTContactsStatus::STATE);
        message.setState(mContacts[id].state);
        message.setIdentity(id);
        message.setNickname(mContacts[id].nickname);
        return send(message);
    }
    return true;
}

bool TTContactsHandler::activate(size_t id) {
    LOG_INFO("Called activate ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) [[unlikely]] {
        return false;
    }

    const auto previousState = mContacts[id].state;
    switch (mContacts[id].state) {
        case TTContactsState::ACTIVE: break;
        case TTContactsState::INACTIVE: mContacts[id].state = TTContactsState::ACTIVE; break;
        case TTContactsState::SELECTED_ACTIVE: break;
        case TTContactsState::SELECTED_INACTIVE: mContacts[id].state = TTContactsState::SELECTED_ACTIVE; break;
        case TTContactsState::UNREAD_MSG_ACTIVE: break;
        case TTContactsState::UNREAD_MSG_INACTIVE: mContacts[id].state = TTContactsState::UNREAD_MSG_ACTIVE; break;
        case TTContactsState::PENDING_MSG_INACTIVE: mContacts[id].state = TTContactsState::ACTIVE; break;
        case TTContactsState::SELECTED_PENDING_MSG_INACTIVE: mContacts[id].state = TTContactsState::SELECTED_ACTIVE; break;
        default:
            LOG_ERROR("Failed to change contact state from {} on activate", size_t(mContacts[id].state));
            return false;
    }

    if (previousState != mContacts[id].state) [[likely]] {
        TTContactsMessage message;
        message.setStatus(TTContactsStatus::STATE);
        message.setState(mContacts[id].state);
        message.setIdentity(id);
        message.setNickname(mContacts[id].nickname);
        return send(message);
    }
    return true;
}

bool TTContactsHandler::deactivate(size_t id) {
    LOG_INFO("Called deactivate ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) [[unlikely]] {
        return false;
    }

    const auto previousState = mContacts[id].state;
    switch (mContacts[id].state) {
        case TTContactsState::ACTIVE: mContacts[id].state = TTContactsState::INACTIVE; break;
        case TTContactsState::INACTIVE: break;
        case TTContactsState::SELECTED_ACTIVE: mContacts[id].state = TTContactsState::SELECTED_INACTIVE; break;
        case TTContactsState::SELECTED_INACTIVE: break;
        case TTContactsState::UNREAD_MSG_ACTIVE: mContacts[id].state = TTContactsState::UNREAD_MSG_INACTIVE; break;
        case TTContactsState::UNREAD_MSG_INACTIVE: break;
        case TTContactsState::PENDING_MSG_INACTIVE: break;
        case TTContactsState::SELECTED_PENDING_MSG_INACTIVE: break;
        default:
            LOG_ERROR("Failed to change contact state from {} on deactivate", size_t(mContacts[id].state));
            return false;
    }

    if (previousState != mContacts[id].state) [[likely]] {
        TTContactsMessage message;
        message.setStatus(TTContactsStatus::STATE);
        message.setState(mContacts[id].state);
        message.setIdentity(id);
        message.setNickname(mContacts[id].nickname);
        return send(message);
    }
    return true;
}

bool TTContactsHandler::select(size_t id) {
    LOG_INFO("Called select ID={}", id);
    std::scoped_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) [[unlikely]] {
        return false;
    }

    // Unselect
    mPreviousContact = mCurrentContact;
    if (mPreviousContact != std::nullopt) {
        const auto previousContactValue = mPreviousContact.value();
        switch (mContacts[previousContactValue].state) {
            case TTContactsState::SELECTED_ACTIVE: mContacts[previousContactValue].state = TTContactsState::ACTIVE; break;
            case TTContactsState::SELECTED_INACTIVE: mContacts[previousContactValue].state = TTContactsState::INACTIVE; break;
            case TTContactsState::SELECTED_PENDING_MSG_INACTIVE: mContacts[previousContactValue].state = TTContactsState::PENDING_MSG_INACTIVE; break;
            case TTContactsState::ACTIVE:
            case TTContactsState::INACTIVE:
            case TTContactsState::UNREAD_MSG_ACTIVE:
            case TTContactsState::UNREAD_MSG_INACTIVE:
            case TTContactsState::PENDING_MSG_INACTIVE:
            default:
                LOG_ERROR("Failed to change contact state from {} on unselect", size_t(mContacts[previousContactValue].state));
                return false;
        }
        TTContactsMessage message;
        message.setStatus(TTContactsStatus::STATE);
        message.setState(mContacts[previousContactValue].state);
        message.setIdentity(previousContactValue);
        message.setNickname(mContacts[previousContactValue].nickname);
        if (!send(message)) {
            return false;
        }
    }
    
    // Select
    mCurrentContact = id;
    switch (mContacts[id].state) {
        case TTContactsState::ACTIVE: mContacts[id].state = TTContactsState::SELECTED_ACTIVE; break;
        case TTContactsState::INACTIVE: mContacts[id].state = TTContactsState::SELECTED_INACTIVE; break;
        case TTContactsState::UNREAD_MSG_ACTIVE: mContacts[id].state = TTContactsState::SELECTED_ACTIVE; break;
        case TTContactsState::UNREAD_MSG_INACTIVE: mContacts[id].state = TTContactsState::SELECTED_INACTIVE; break;
        case TTContactsState::PENDING_MSG_INACTIVE: mContacts[id].state = TTContactsState::SELECTED_PENDING_MSG_INACTIVE; break;
        case TTContactsState::SELECTED_ACTIVE:
        case TTContactsState::SELECTED_INACTIVE:
        case TTContactsState::SELECTED_PENDING_MSG_INACTIVE:
        default:
            LOG_ERROR("Failed to change contact state from {} on select", size_t(mContacts[id].state));
            return false;
    }
    TTContactsMessage message;
    message.setStatus(TTContactsStatus::STATE);
    message.setState(mContacts[id].state);
    message.setIdentity(id);
    message.setNickname(mContacts[id].nickname);
    return send(message);
}

std::optional<TTContactsHandlerEntry> TTContactsHandler::get(size_t id) const {
    LOG_INFO("Called get ID={}", id);
    std::shared_lock contactsLock(mContactsMutex);
    if (id >= mContacts.size()) [[unlikely]] {
        return std::nullopt;
    }
    return {mContacts[id]};
}

std::optional<size_t> TTContactsHandler::get(const std::string& id) const {
    LOG_INFO("Called get ID={}", id);
    std::shared_lock contactsLock(mContactsMutex);
    decltype(auto) result = mIdentityMap.find(id);
    if (result == mIdentityMap.end()) [[unlikely]] {
        return std::nullopt;
    }
    return {result->second};
}

std::optional<size_t> TTContactsHandler::current() const {
    std::shared_lock contactsLock(mContactsMutex);
    LOG_INFO("Called current");
    return mCurrentContact;
}

size_t TTContactsHandler::size() const {
    std::shared_lock contactsLock(mContactsMutex);
    return mContacts.size();
}

void TTContactsHandler::onStop() {
    mQueueCondition.notify_one();
}

bool TTContactsHandler::send(const TTContactsMessage& message) {
    if (isStopped()) {
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
        while (!isStopped()) {
            TTContactsMessage message;
            message.setStatus(TTContactsStatus::HEARTBEAT);
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
                    return !mQueuedMessages.empty() || isStopped();
                });

                if (isStopped()) {
                    exit = true;
                    goodbye();
                    break; // Forced exit
                }

                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
            }

            for (auto &message : messages) {
                if (isStopped()) {
                    exit = true;
                    goodbye();
                    break; // Forced exit
                }
                if (!mSharedMem->send(reinterpret_cast<const void*>(message.get()))) {
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
    message.setStatus(TTContactsStatus::HEARTBEAT);
    return mSharedMem->send(reinterpret_cast<const void*>(&message), 5);
}

void TTContactsHandler::goodbye() {
    LOG_WARNING("Sending goodbye message...");
    TTContactsMessage message;
    message.setStatus(TTContactsStatus::GOODBYE);
    mSharedMem->send(reinterpret_cast<const void*>(&message));
}
