#include "TTChatHandler.hpp"
#include "TTChatSettings.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <list>
#include <limits>
#include <iostream>

TTChatHandler::TTChatHandler(const TTChatSettings& settings) :
        mPrimaryMessageQueue(settings.getPrimaryMessageQueue()),
        mSecondaryMessageQueue(settings.getSecondaryMessageQueue()),
        mStopped{false},
        mHeartbeatResult{},
        mHandlerResult{},
        mCurrentId(std::nullopt) {
    LOG_INFO("Constructing...");
    if (!mPrimaryMessageQueue->create()) {
        throw std::runtime_error("TTChatHandler: Failed to create primary message queue!");
    }
    if (!mSecondaryMessageQueue->create()) {
        throw std::runtime_error("TTChatHandler: Failed to create secondary message queue!");
    }
    // Set heartbeat receiver thread
    auto pt = std::packaged_task<void()>(std::bind(&TTChatHandler::heartbeat, this));
    mHeartbeatResult = pt.get_future();
    mHeartbeatThread = std::thread(std::move(pt));
    mHeartbeatThread.detach();
    // Set handler thread
    mHandlerResult = std::async(std::launch::async, std::bind(&TTChatHandler::main, this));
    LOG_INFO("Successfully constructed!");
}

TTChatHandler::~TTChatHandler() {
    LOG_INFO("Destructing...");
    stop();
    if (mHeartbeatResult.valid()) {
        mHeartbeatResult.wait();
    }
    if (mHandlerResult.valid()) {
        mHandlerResult.wait();
    }
    LOG_INFO("Successfully destructed!");
}

bool TTChatHandler::send(size_t id, const std::string& message, TTChatTimestamp timestamp) {
    if (stopped()) {
        LOG_WARNING("Forced exit on send!");
        return false;
    }
    std::scoped_lock messagesLock(mMessagesMutex);
    if (mCurrentId == std::nullopt || mCurrentId.value() != id) {
        LOG_ERROR("Current ID is either null or not matching the ID={} on send!", id);
        return false;
    }
    if (id >= mMessages.size()) {
        LOG_ERROR("ID={} out of range on send!", id);
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back({TTChatMessageType::SENDER, timestamp, message});
    if (!send(TTChatMessageType::SENDER, message, timestamp)) {
        return false;
    }
    LOG_INFO("Successfully updated storage with new send message type, ID={}", id);
    return true;
}

bool TTChatHandler::receive(size_t id, const std::string& message, TTChatTimestamp timestamp) {
    if (stopped()) {
        LOG_WARNING("Forced exit on receive!");
        return false;
    }
    std::scoped_lock messagesLock(mMessagesMutex);
    if (id >= mMessages.size()) {
        LOG_ERROR("ID={} out of range on receive!", id);
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back({TTChatMessageType::RECEIVER, timestamp, message});
    if (mCurrentId && mCurrentId.value() == id) {
        if (!send(TTChatMessageType::RECEIVER, message, timestamp)) {
            return false;
        }
    }
    LOG_INFO("Successfully updated storage with new receive message type, ID={}", id);
    return true;
}

bool TTChatHandler::select(size_t id) {
    if (stopped()) {
        LOG_WARNING("Forced exit on select!");
        return false;
    }
    std::scoped_lock messagesLock(mMessagesMutex);
    if (id >= mMessages.size()) {
        LOG_ERROR("ID={} out of range on select!", id);
        return false;
    }
    if (mCurrentId && mCurrentId.value() == id) {
        LOG_WARNING("Current ID={} is matching, no need continue on select!", id);
        return true;
    }
    if (mCurrentId) {
        if (!send(TTChatMessageType::CLEAR, {}, std::chrono::system_clock::now())) {
            return false;
        }
    }
    mCurrentId = id;
    const auto& storage = mMessages[id];
    for (const auto &message : storage) {
        if (!send(message.type, message.data, message.timestamp)) {
            return false;
        }
    }
    LOG_INFO("Successfully selected new ID={}", id);
    return true;
}

bool TTChatHandler::create(size_t id) {
    if (stopped()) {
        LOG_WARNING("Forced exit on create!");
        return false;
    }
    std::scoped_lock messagesLock(mMessagesMutex);
    if (id != mMessages.size()) {
        LOG_ERROR("ID={} is within existing range boundaries on create!", id);
        return false;
    }
    // New storage
    mMessages.push_back({});
    LOG_INFO("Successfully created new storage, ID={}", id);
    return true;
}

bool TTChatHandler::size() const {
    std::shared_lock messagesLock(mMessagesMutex);
    return mMessages.size();
}

std::optional<TTChatEntries> TTChatHandler::get(size_t id) const {
    std::shared_lock messagesLock(mMessagesMutex);
    if (id >= mMessages.size()) {
        LOG_ERROR("Failed to return messages of ID={}", id);
        return std::nullopt;
    }
    return {mMessages[id]};
}

std::optional<size_t> TTChatHandler::current() const {
    std::shared_lock messagesLock(mMessagesMutex);
    return mCurrentId;
}

void TTChatHandler::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
    mQueueCondition.notify_one();
}

bool TTChatHandler::stopped() const {
    return mStopped.load();
}

bool TTChatHandler::send(TTChatMessageType type, const std::string& data, TTChatTimestamp timestamp) {
    LOG_INFO("Started preparing messages to be queued");
    if (stopped()) {
        LOG_WARNING("Forced exit at generic message type!");
        return false;
    }

    size_t numberOfFullMessages = (data.size() / TTChatMessage::MAX_DATA_LENGTH);
    size_t totalFullMessagesDataLength = numberOfFullMessages * TTChatMessage::MAX_DATA_LENGTH;
    size_t lastMessageDataLength = data.size() - totalFullMessagesDataLength;

    if (numberOfFullMessages != 0 && lastMessageDataLength == 0) {
        numberOfFullMessages -= 1;
        totalFullMessagesDataLength -= TTChatMessage::MAX_DATA_LENGTH;
        lastMessageDataLength = TTChatMessage::MAX_DATA_LENGTH;
    }

    // Create chunk messages
    std::list<std::unique_ptr<TTChatMessage>> messages;
    for (size_t i = 0; i < numberOfFullMessages; ++i) {
        const char* cdata = data.c_str() + (TTChatMessage::MAX_DATA_LENGTH * i);
        const auto chunkType = static_cast<TTChatMessageType>(static_cast<size_t>(type) + 1);
        auto message = std::make_unique<TTChatMessage>(chunkType, timestamp, std::string_view(cdata, TTChatMessage::MAX_DATA_LENGTH));
        messages.push_back(std::move(message));
    }
    // Create full message
    {
        const char* cdata = data.c_str() + totalFullMessagesDataLength;
        auto message = std::make_unique<TTChatMessage>(type, timestamp, std::string_view(cdata, lastMessageDataLength));
        messages.push_back(std::move(message));
    }
    // Fill the queue
    {
        std::scoped_lock lock(mQueueMutex);
        for (auto & it : messages) {
            mQueuedMessages.push(std::move(it));
        }
    }
    mQueueCondition.notify_one();
    LOG_INFO("Completed preparing messages to be queued");
    return true;
    
}

std::list<std::unique_ptr<TTChatMessage>> TTChatHandler::dequeue() {
    LOG_INFO("Started dequeue");
    std::list<std::unique_ptr<TTChatMessage>> messages;
    while (true)
    {
        bool predicate = true;
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            predicate = mQueueCondition.wait_for(lock, mHeartbeatTimeout, [this]() {
                return !mQueuedMessages.empty() || stopped();
            });

            if (stopped()) {
                LOG_WARNING("Forced exit on dequeue");
                throw std::runtime_error({});
            }
            
            if (!mQueuedMessages.empty()) {
                LOG_INFO("Inserting queued messages...");
                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
            } else {
                LOG_INFO("No queued messages");
            }
            // Do not remove scope guards, risk of deadlock
        }
        if (!predicate) {
            // There wasn't a new message in a queue
            LOG_INFO("Inserting heartbeat message...");
            if (!send(TTChatMessageType::HEARTBEAT, {}, std::chrono::system_clock::now())) {
                throw std::runtime_error({});
            }
            continue;
        }
        break;
    }
    LOG_INFO("Completed dequeue");
    return messages;
}

void TTChatHandler::heartbeat() {
    LOG_INFO("Started secondary (heartbeat) loop");
    if (!mPrimaryMessageQueue->alive() || !mSecondaryMessageQueue->alive()) {
        LOG_ERROR("Primary or secondary message queue is not alive!");
    } else {
        try {
            TTChatMessage message;
            while (true) {
                if (stopped()) {
                    LOG_WARNING("Forced exit on secondary (heartbeat) loop");
                    break;
                }
                if (!mSecondaryMessageQueue->receive(reinterpret_cast<char*>(&message))) {
                    LOG_ERROR("Failed to receive heartbeat message!");
                    break;
                }
                if (message.getType() != TTChatMessageType::HEARTBEAT) {
                    LOG_ERROR("Received message other than the heartbeat message!");
                    break;
                }
            }
        } catch (...) {
            LOG_ERROR("Caught unknown exception at secondary (heartbeat) loop!");
        }
    }
    stop();
    LOG_INFO("Completed secondary (heartbeat) loop");
}

void TTChatHandler::main() {
    LOG_INFO("Started primary loop");
    if (!mPrimaryMessageQueue->alive() || !mSecondaryMessageQueue->alive()) {
        LOG_ERROR("Primary or secondary message queue is not alive!");
    } else {
        try {
            bool exit = false;
            while (!exit) {
                decltype(auto) messages = dequeue();
                for (auto &message : messages) {
                    auto& refMessage = *message.get();
                    if (stopped()) {
                        LOG_WARNING("Forced exit on primary loop");
                        exit = true;
                        break;
                    }
                    if (!mPrimaryMessageQueue->send(reinterpret_cast<const char*>(&refMessage))) {
                        LOG_WARNING("Failed to send message!");
                        exit = true;
                        break;
                    }
                }
            }
        } catch (...) {
            LOG_ERROR("Caught unknown exception at primary loop!");
        }
        goodbye();
    }
    stop();
    LOG_INFO("Completed primary loop");
}

void TTChatHandler::goodbye() {
    LOG_WARNING("Sending goodbye message...");
    TTChatMessage message;
    message.setType(TTChatMessageType::GOODBYE);
    mPrimaryMessageQueue->send(reinterpret_cast<const char*>(&message));
}
