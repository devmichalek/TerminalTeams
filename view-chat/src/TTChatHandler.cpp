#include "TTChatHandler.hpp"
#include <list>
#include <limits>
#include <iostream>

TTChatHandler::TTChatHandler(std::string messageQueueName,
    TTChatCallbackMessageSent callbackMessageSent,
    TTChatCallbackMessageReceived callbackMessageReceived) :
        mCallbackMessageSent(callbackMessageSent),
		mCallbackMessageReceived(callbackMessageReceived),
        mMessageQueueName(messageQueueName),
        mMessageQueueDescriptor(-1),
        mMessageQueueReversedName(mMessageQueueName + "-reversed"),
        mMessageQueueReversedDescriptor(-1),
        mForcedQuit{false},
        mHeartbeatResult{},
        mHandlerResult{},
        mCurrentId(std::numeric_limits<size_t>::max()) {
    const std::string classNamePrefix = "TTChatHandler: ";
    // Correct queue name
    if (mMessageQueueName.front() != '/') {
        mMessageQueueName.insert(0, "/");
        mMessageQueueReversedName.insert(0, "/");
    }
    // Create and open message queue
	struct mq_attr messageQueueAttributes;
	messageQueueAttributes.mq_maxmsg = TTCHAT_MESSAGE_MAX_NUM;
    messageQueueAttributes.mq_msgsize = TTCHAT_MESSAGE_MAX_LENGTH;
	messageQueueAttributes.mq_flags = 0;

	errno = 0;
	mMessageQueueDescriptor = mq_open(mMessageQueueName.c_str(),
                                      O_CREAT | O_RDWR,
                                      0644,
                                      &messageQueueAttributes);
	if (mMessageQueueDescriptor == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to create message queue, errno=" + std::to_string(errno));
	}
    // Create and open message queue reversed
    errno = 0;
	mMessageQueueReversedDescriptor = mq_open(mMessageQueueReversedName.c_str(),
                                              O_CREAT | O_RDWR,
                                              0644,
                                              &messageQueueAttributes);
	if (mMessageQueueReversedDescriptor == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to create reversed message queue, errno=" + std::to_string(errno));
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
    mForcedQuit.store(true);
    mQueueCondition.notify_one();
    mHeartbeatResult.wait();
    mHandlerResult.wait();
    mq_unlink(mMessageQueueReversedName.c_str());
    mq_unlink(mMessageQueueName.c_str());
}

bool TTChatHandler::send(size_t id, std::string message, TTChatTimestamp timestamp) {
    if (id >= mMessages.size()) {
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back(std::make_tuple(TTChatMessageType::SEND, message, timestamp));
    if (mCurrentId == id) {
        return send(TTChatMessageType::SEND, message, timestamp);
    }
    if (mForcedQuit.load()) {
        return false;
    }
    return true;
}

bool TTChatHandler::receive(size_t id, std::string message, TTChatTimestamp timestamp) {
    if (id >= mMessages.size()) {
        return false;
    }
    auto& storage = mMessages[id];
    storage.push_back(std::make_tuple(TTChatMessageType::RECEIVE, message, timestamp));
    if (mCurrentId == id) {
        return send(TTChatMessageType::RECEIVE, message, timestamp);
    }
    if (mForcedQuit.load()) {
        return false;
    }
    return true;
}

bool TTChatHandler::clear(size_t id) {
    if (id >= mMessages.size()) {
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
    return true;
}

bool TTChatHandler::create(size_t id) {
    if (mForcedQuit.load()) {
        return false;
    }
    if (id < mMessages.size()) {
        return false;
    }
    // New storage
    mMessages.push_back({});
    return true;
}

const TTChatEntries& TTChatHandler::get(size_t id) {
    if (id >= mMessages.size()) {
        throw std::runtime_error("TTChatHandler: Failed to return messages of id=" + std::to_string(id));
    }
    return mMessages[id];
}

bool TTChatHandler::send(TTChatMessageType type, std::string data, TTChatTimestamp timestamp) {
    if (mForcedQuit.load()) {
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
    return true;
    
}

void TTChatHandler::heartbeat() {
    try {
        char dummyBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
        for (auto i = TTCHAT_MESSAGE_RECEIVE_TRY_COUNT; i >= 0; --i) {
            if (mForcedQuit.load()) {
                // Forced exit
                throw std::runtime_error({});
            }
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                // Hard failure
                throw std::runtime_error({});
            }
            ts.tv_sec += TTCHAT_MESSAGE_RECEIVE_TIMEOUT_S;
            errno = 0;
            unsigned int priority = 0;
            auto result = mq_timedreceive(mMessageQueueReversedDescriptor,
                                          dummyBuffer,
                                          TTCHAT_MESSAGE_MAX_LENGTH,
                                          &priority,
                                          &ts);
            if (result != -1) {
                i = TTCHAT_MESSAGE_RECEIVE_TRY_COUNT + 1;
                continue;
            }
            if (errno == EAGAIN || errno == ETIMEDOUT) {
                continue;
            }
            // Hard failure
            throw std::runtime_error({});
        }
    } catch (...) {
        // ...
    }
    mForcedQuit.store(true);
}

void TTChatHandler::main() {
    try {
        while (true) {
            // Fill the list of messages
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
                        throw std::runtime_error({});
                    }
                    while (!mQueuedMessages.empty()) {
                        messages.push_back(std::move(mQueuedMessages.front()));
                        mQueuedMessages.pop();
                    }
                    // Do not remove scope guards, risk of deadlock
                }

                if (!predicate) {
                    // There wasn't a new message in a queue
                    if (!send(TTChatMessageType::HEARTBEAT, {}, std::chrono::system_clock::now())) {
                        break; // Fail
                    }
                    continue;
                }

                break;
            }

            for (auto &message : messages) {
                int result = -1;
                auto& refMessage = *message.get();
                for (auto i = TTCHAT_MESSAGE_SEND_TRY_COUNT; i >= 0; --i) {
                    if (mForcedQuit.load()) {
                        throw std::runtime_error({});
                    }
                    struct timespec ts;
                    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        throw std::runtime_error({}); // Hard failure
                    }
                    ts.tv_sec += TTCHAT_MESSAGE_SEND_TIMEOUT_S;
                    errno = 0;
                    result = mq_timedsend(mMessageQueueDescriptor,
                                          reinterpret_cast<char*>(&refMessage),
                                          TTCHAT_MESSAGE_MAX_LENGTH,
                                          TTCHAT_MESSAGE_PRIORITY,
                                          &ts);
                    if (result != -1) {
                        if (refMessage.type == TTChatMessageType::SEND) {
                            if (mCallbackMessageSent) {
                                mCallbackMessageSent();
                            }
                        } else if (refMessage.type == TTChatMessageType::RECEIVE) {
                            if (mCallbackMessageReceived) {
                                mCallbackMessageReceived();
                            }
                        }
                        break; // Success
                    }
                    if (errno == EAGAIN) {
                        continue; // Message queue is full, try again
                    }
                    if (errno == ETIMEDOUT) {
                        continue; // Timeout, try again
                    }
                    // Hard failure
                    throw std::runtime_error({});
                }

                if (result == -1) {
                    // Hard failure
                    throw std::runtime_error({});
                }
            }
        }
    } catch (...) {
        // ...
    }
    mForcedQuit.store(true);
}
