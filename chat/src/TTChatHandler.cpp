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
        mCurrentId(std::numeric_limits<size_t>::max()) {
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
    mQueueCondition.notify_one();
    mHeartbeatResult.wait();
    mHandlerResult.wait();
    LOG_INFO("Successfully destructed!");
}

bool TTChatHandler::send(size_t id, std::string message, TTChatTimestamp timestamp) {
    if (stopped()) {
        LOG_WARNING("Forced exit at send message type!");
        return false;
    }
    std::shared_lock messagesLock(mMessagesMutex);
    if (id >= mMessages.size()) {
        LOG_ERROR("ID={} out of range at send message type!", id);
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back(std::make_tuple(TTChatMessageType::SEND, message, timestamp));
    std::shared_lock currentIdLock(mCurrentIdMutex);
    if (mCurrentId == id) {
        if (!send(TTChatMessageType::SEND, message, timestamp)) {
            return false;
        }
    }
    LOG_INFO("Successfully updated storage with new send message type, ID={}", id);
    return true;
}

bool TTChatHandler::receive(size_t id, std::string message, TTChatTimestamp timestamp) {
    if (stopped()) {
        LOG_WARNING("Forced exit at receive message type!");
        return false;
    }
    std::shared_lock messagesLock(mMessagesMutex);
    if (id >= mMessages.size()) {
        LOG_ERROR("ID={} out of range at receive message type!", id);
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back(std::make_tuple(TTChatMessageType::RECEIVE, message, timestamp));
    std::shared_lock currentIdLock(mCurrentIdMutex);
    if (mCurrentId == id) {
        if (!send(TTChatMessageType::RECEIVE, message, timestamp)) {
            return false;
        }
    }
    LOG_INFO("Successfully updated storage with new receive message type, ID={}", id);
    return true;
}

bool TTChatHandler::clear(size_t id) {
    if (stopped()) {
        LOG_WARNING("Forced exit at clear message type!");
        return false;
    }
    std::shared_lock messagesLock(mMessagesMutex);
    if (id >= mMessages.size()) {
        LOG_ERROR("ID={} out of range at clear message type!", id);
        return false;
    }
    if (!send(TTChatMessageType::CLEAR, {}, std::chrono::system_clock::now())) {
        return false;
    }
    std::scoped_lock currentIdLock(mCurrentIdMutex);
    mCurrentId = id;
    auto& storage = mMessages[id];
    for (auto &message : storage) {
        if (!send(std::get<0>(message), std::get<1>(message), std::get<2>(message))) {
            return false;
        }
    }
    LOG_INFO("Successfully cleared and updated current ID={}", id);
    return true;
}

bool TTChatHandler::create(size_t id) {
    if (stopped()) {
        LOG_WARNING("Forced exit at create message type!");
        return false;
    }
    std::scoped_lock messagesLock(mMessagesMutex);
    if (!mMessages.empty() && id < mMessages.size()) {
        LOG_ERROR("ID={} is within existing range!", id);
        return false;
    }
    // New storage
    mMessages.push_back({});
    LOG_INFO("Successfully created new storage, ID={}", id);
    return true;
}

const TTChatEntries& TTChatHandler::get(size_t id) {
    if (id >= mMessages.size()) {
        LOG_ERROR("Failed to return messages of ID={}", id);
        throw std::runtime_error({});
    }
    return mMessages[id];
}

void TTChatHandler::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTChatHandler::stopped() const {
    return mStopped.load();
}

bool TTChatHandler::send(TTChatMessageType type, std::string data, TTChatTimestamp timestamp) {
    LOG_INFO("Started preparing messages to be queued");
    if (stopped()) {
        LOG_WARNING("Forced exit at generic message type!");
        return false;
    }

    std::list<std::unique_ptr<TTChatMessage>> messages;
    const size_t numberOfFullMessages = (data.size() / TTCHAT_DATA_MAX_LENGTH);
    for (size_t i = 0; i < numberOfFullMessages; ++i) {
        const char* cdata = data.c_str() + (TTCHAT_DATA_MAX_LENGTH * i);
        auto message = std::make_unique<TTChatMessage>(type, timestamp, TTCHAT_DATA_MAX_LENGTH, cdata);
        messages.push_back(std::move(message));
    }

    const size_t totalFullMessagesDataLength = numberOfFullMessages * TTCHAT_DATA_MAX_LENGTH;
    const size_t lastMessageDataLength = data.size() - totalFullMessagesDataLength;
    if (lastMessageDataLength > 0 || data.empty()) {
        const char* cdata = data.c_str() + totalFullMessagesDataLength;
        auto message = std::make_unique<TTChatMessage>(type, timestamp, lastMessageDataLength, cdata);
        messages.push_back(std::move(message));
    }

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
            auto waitTimeMs = std::chrono::milliseconds(TTCHAT_HEARTBEAT_TIMEOUT_MS);
            predicate = mQueueCondition.wait_for(lock, waitTimeMs, [this]() {
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
    if (mPrimaryMessageQueue->alive() && mSecondaryMessageQueue->alive()) { 
        try {
            char dummyBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
            while (true) {
                if (stopped()) {
                    LOG_WARNING("Forced exit on secondary (heartbeat) loop");
                    break;
                }
                if (!mSecondaryMessageQueue->receive(reinterpret_cast<char*>(&dummyBuffer[0]))) {
                    LOG_WARNING("Failed to receive heartbeat message!");
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(TTCHAT_HEARTBEAT_TIMEOUT_MS));
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
    if (mPrimaryMessageQueue->alive() && mSecondaryMessageQueue->alive()) {
        try {
            while (true) {
                decltype(auto) messages = dequeue();
                for (auto &message : messages) {
                    auto& refMessage = *message.get();
                    if (stopped()) {
                        LOG_WARNING("Forced exit on primary loop");
                        throw std::runtime_error({});
                    }
                    if (!mPrimaryMessageQueue->send(reinterpret_cast<char*>(&refMessage))) {
                        LOG_WARNING("Failed to send message!");
                        throw std::runtime_error({});
                    }
                }
            }
        } catch (...) {
            TTChatMessage goodbye(TTChatMessageType::GOODBYE, std::chrono::system_clock::now(), 0, nullptr);
            mPrimaryMessageQueue->send(reinterpret_cast<char*>(&goodbye));
            LOG_ERROR("Caught unknown exception at primary loop!");
        }
    }
    stop();
    LOG_INFO("Completed primary loop");
}
