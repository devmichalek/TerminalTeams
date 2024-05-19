#include "TTChatHandler.hpp"
#include "TTChatSettings.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <list>
#include <limits>
#include <iostream>

TTChatHandler::TTChatHandler(const TTChatSettings& settings) :
        mPrimaryMessageQueue(settings.getPrimaryMessageQueue()),
        mSecondaryMessageQueue(settings.getSecondaryMessageQueue()),
        mForcedQuit{false},
        mHeartbeatResult{},
        mHandlerResult{},
        mCurrentId(std::numeric_limits<size_t>::max()) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Constructing...", mClassNamePrefix);
    if (!mPrimaryMessageQueue->create()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to create primary message queue!");
    } else {
        logger.info("{} Successfully created primary message queue!", mClassNamePrefix);
    }
    if (!mSecondaryMessageQueue->create()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to create secondary message queue!");
    } else {
        logger.info("{} Successfully created secondary message queue!", mClassNamePrefix);
    }

    // Set heartbeat receiver thread
    auto pt = std::packaged_task<void()>(std::bind(&TTChatHandler::heartbeat, this));
    mHeartbeatResult = pt.get_future();
    mHeartbeatThread = std::thread(std::move(pt));
    mHeartbeatThread.detach();
    // Set handler thread
    mHandlerResult = std::async(std::launch::async, std::bind(&TTChatHandler::main, this));
}

TTChatHandler::~TTChatHandler() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    mForcedQuit.store(true);
    mQueueCondition.notify_one();
    mHeartbeatResult.wait();
    mHandlerResult.wait();
}

bool TTChatHandler::send(size_t id, std::string message, TTChatTimestamp timestamp) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (mForcedQuit.load()) {
        logger.warning("{} Forced exit at send message type!", mClassNamePrefix);
        return false;
    }
    if (id >= mMessages.size()) {
        logger.error("{} ID={} out of range at send message type!", mClassNamePrefix, id);
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back(std::make_tuple(TTChatMessageType::SEND, message, timestamp));
    if (mCurrentId == id) {
        if (!send(TTChatMessageType::SEND, message, timestamp)) {
            return false;
        }
    }
    logger.info("{} Successfully updated storage with new send message type, ID={}", mClassNamePrefix, id);
    return true;
}

bool TTChatHandler::receive(size_t id, std::string message, TTChatTimestamp timestamp) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (mForcedQuit.load()) {
        logger.warning("{} Forced exit at receive message type!", mClassNamePrefix);
        return false;
    }
    if (id >= mMessages.size()) {
        logger.error("{} ID={} out of range at receive message type!", mClassNamePrefix, id);
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back(std::make_tuple(TTChatMessageType::RECEIVE, message, timestamp));
    if (mCurrentId == id) {
        if (!send(TTChatMessageType::RECEIVE, message, timestamp)) {
            return false;
        }
    }
    logger.info("{} Successfully updated storage with new receive message type, ID={}", mClassNamePrefix, id);
    return true;
}

bool TTChatHandler::clear(size_t id) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (mForcedQuit.load()) {
        logger.warning("{} Forced exit at clear message type!", mClassNamePrefix);
        return false;
    }
    if (id >= mMessages.size()) {
        logger.error("{} ID={} out of range at clear message type!", mClassNamePrefix, id);
        return false;
    }
    if (!send(TTChatMessageType::CLEAR, {}, std::chrono::system_clock::now())) {
        return false;
    }
    mCurrentId = id;
    auto& storage = mMessages[id];
    for (auto &message : storage) {
        if (!send(std::get<0>(message), std::get<1>(message), std::get<2>(message))) {
            return false;
        }
    }
    logger.info("{} Successfully cleared and updated current ID={}", mClassNamePrefix, id);
    return true;
}

bool TTChatHandler::create(size_t id) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (mForcedQuit.load()) {
        logger.warning("{} Forced exit at create message type!", mClassNamePrefix);
        return false;
    }
    if (!mMessages.empty() && id < mMessages.size()) {
        logger.error("{} ID={} is within existing range!", mClassNamePrefix, id);
        return false;
    }
    // New storage
    mMessages.push_back({});
    logger.info("{} Successfully created new storage, ID={}", mClassNamePrefix, id);
    return true;
}

const TTChatEntries& TTChatHandler::get(size_t id) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (id >= mMessages.size()) {
        logger.error("{} Failed to return messages of ID={}", mClassNamePrefix, id);
        throw std::runtime_error({});
    }
    return mMessages[id];
}

bool TTChatHandler::send(TTChatMessageType type, std::string data, TTChatTimestamp timestamp) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started preparing messages to be queued", mClassNamePrefix);
    if (mForcedQuit.load()) {
        logger.warning("{} Forced exit at generic message type!", mClassNamePrefix);
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
        std::scoped_lock<std::mutex> lock(mQueueMutex);
        for (auto & it : messages) {
            mQueuedMessages.push(std::move(it));
        }
    }
    mQueueCondition.notify_one();
    logger.info("{} Completed preparing messages to be queued", mClassNamePrefix);
    return true;
    
}

std::list<std::unique_ptr<TTChatMessage>> TTChatHandler::dequeue() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started dequeue", mClassNamePrefix);
    std::list<std::unique_ptr<TTChatMessage>> messages;
    while (true)
    {
        bool predicate = true;
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            auto waitTimeMs = std::chrono::milliseconds(TTCHAT_HEARTBEAT_TIMEOUT_MS);
            predicate = mQueueCondition.wait_for(lock, waitTimeMs, [this]() {
                return !mQueuedMessages.empty() || mForcedQuit.load();
            });

            if (mForcedQuit.load()) {
                logger.warning("{} Forced exit on dequeue", mClassNamePrefix);
                throw std::runtime_error({});
            }
            
            if (!mQueuedMessages.empty()) {
                logger.info("{} Inserting queued messages...", mClassNamePrefix);
                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
            } else {
                logger.info("{} No queued messages", mClassNamePrefix);
            }
            // Do not remove scope guards, risk of deadlock
        }

        if (!predicate) {
            // There wasn't a new message in a queue
            logger.info("{} Inserting heartbeat message...", mClassNamePrefix);
            if (!send(TTChatMessageType::HEARTBEAT, {}, std::chrono::system_clock::now())) {
                throw std::runtime_error({});
            }
            continue;
        }

        break;
    }
    logger.info("{} Completed dequeue", mClassNamePrefix);
    return messages;
}

void TTChatHandler::heartbeat() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started secondary (heartbeat) loop", mClassNamePrefix);
    if (mPrimaryMessageQueue->alive() && mSecondaryMessageQueue->alive()) { 
        try {
            char dummyBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
            while (true) {
                if (mForcedQuit.load()) {
                    logger.warning("{} Forced exit on secondary (heartbeat) loop", mClassNamePrefix);
                    break;
                }
                if (!mSecondaryMessageQueue->receive(reinterpret_cast<char*>(&dummyBuffer[0]))) {
                    logger.warning("{} Failed to receive heartbeat message!", mClassNamePrefix);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(TTCHAT_HEARTBEAT_TIMEOUT_MS));
            }
        } catch (...) {
            logger.error("{} Caught unknown exception at secondary (heartbeat) loop!", mClassNamePrefix);
        }
    }
    mForcedQuit.store(true);
    logger.info("{} Completed secondary (heartbeat) loop", mClassNamePrefix);
}

void TTChatHandler::main() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started primary loop", mClassNamePrefix);
    if (mPrimaryMessageQueue->alive() && mSecondaryMessageQueue->alive()) {
        try {
            while (true) {
                decltype(auto) messages = dequeue();
                for (auto &message : messages) {
                    auto& refMessage = *message.get();
                    if (mForcedQuit.load()) {
                        logger.warning("{} Forced exit on primary loop", mClassNamePrefix);
                        throw std::runtime_error({});
                    }
                    if (!mPrimaryMessageQueue->send(reinterpret_cast<char*>(&refMessage))) {
                        logger.warning("{} Failed to send message!", mClassNamePrefix);
                        throw std::runtime_error({});
                    }
                }
            }
        } catch (...) {
            logger.error("{} Caught unknown exception at primary loop!", mClassNamePrefix);
        }
    }
    mForcedQuit.store(true);
    logger.info("{} Completed primary loop", mClassNamePrefix);
}
