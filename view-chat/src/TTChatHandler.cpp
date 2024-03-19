#include "TTChatHandler.hpp"
#include <functional>
#include <list>

TTChatHandler::TTChatHandler(std::string messageQueueName) :
    mMessageQueueName(messageQueueName),
    mMessageQueueDescriptor(-1),
    mMessageQueueReversedName(mMessageQueueName + "-reversed"),
    mMessageQueueReversedDescriptor(-1),
    mForcedQuit{false},
    mHeartbeatReceiverResult{},
    mHeartbeatSenderResult{},
    mHandlerResult{} {
    const std::string classNamePrefix = "TTChatHandler: ";

	struct mq_attr messageQueueAttributes;
	messageQueueAttributes.mq_maxmsg = TTCHAT_MESSAGE_MAX_NUM;
	
	messageQueueAttributes.mq_flags = 0;

	errno = 0;
	mMessageQueueDescriptor = mq_open(mMessageQueueName.c_str(),
                                      O_CREAT | O_RDWR | O_NONBLOCK,
                                      0644,
                                      &attributes);
	if (mMessageQueueDescriptor == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to open message queue, errno=" + std::to_string(errno));
	}

    errno = 0;
    messageQueueAttributes.mq_msgsize = 0;
	mMessageQueueReversedDescriptor = mq_open(mMessageQueueReversedName.c_str(),
                                              O_CREAT | O_RDWR | O_NONBLOCK,
                                              0644,
                                              &attributes);
	if (mMessageQueueReversedDescriptor == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to open reversed message queue, errno=" + std::to_string(errno));
	}

    // Set heartbeat receiver thread
    auto pt = std::packaged_task<void()>(std::bind(&TTChatHandler::heartbeat, this));
    mHeartbeatResult = pt.get_future();
    std::thread(std::move(pt)).detach();
    // Set handler thread
    mHandlerResult = std::async(std::launch::async, &TTChatHandler::main);
}

TTChatHandler::~TTChatHandler() {
    mForcedQuit.store(true);
    mQueueCondition.notify_one();
    mHeartbeatResult.wait();
    mHandlerResult.wait();
    mq_unlink(mMessageQueueReversedName.c_str());
    mq_unlink(mMessageQueueName.c_str());
}

bool TTChatHandler::send(std::string message, TTChatTimestamp timestamp) {
    return send(TTChatMessageType::SEND, message, timestamp);
}

bool TTChatHandler::receive(std::string message, TTChatTimestamp timestamp) {
    return send(TTChatMessageType::RECEIVE, message, timestamp);
}

bool TTChatHandler::clear() {
    return send(TTChatMessageType::CLEAR, {}, std::chrono::system_clock::now());
}

bool TTChatHandler::send(TTChatMessageType type, std::string data, TTChatTimestamp timestamp) {
    if (mForcedQuit.load()) {
        return false;
    }

    std::list<TTChatMessage> messages;
    const size_t numberOfFullMessages = (data.size() / TTCHAT_DATA_MAX_LENGTH);
    for (size_t i = 0; i < numberOfFullMessages; ++i) {
        const char* data = data.c_str() + (TTCHAT_DATA_MAX_LENGTH * i);
        auto message = std::make_unique<TTChatMessage>(type, timestamp, TTCHAT_DATA_MAX_LENGTH, data);
        messages.push_back(std::move(message));
    }

    const size_t totalFullMessagesDataLength = numberOfFullMessages * TTCHAT_DATA_MAX_LENGTH;
    const size_t lastMessageDataLength = data.size() - totalFullMessagesDataLength;
    if (lastMessageDataLength > 0) {
        const char* data = data.c_str() + totalFullMessagesDataLength;
        auto message = std::make_unique<TTChatMessage>(type, timestamp, lastMessageDataLength, data);
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

void TTChatHandler::hearbeat() {
    try {
        char dummyBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
        for (auto i = TTCHAT_MESSAGE_RECEIVE_TRY_COUNT; i >= 0 && !mForcedQuit.load(); --i) {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                break; // Hard failure
            }
            ts.tv_sec += TTCHAT_MESSAGE_RECEIVE_TIMEOUT_S;
            errno = 0;
            unsigned int priority = 0;
            auto result = mq_timedreceive(mMessageQueueReversedDescriptor,
                                          dummyBuffer,
                                          TTCHAT_MESSAGE_MAX_LENGTH,
                                          &priority,
                                          &ts);
            if (result == 0) {
                i = TTCHAT_MESSAGE_RECEIVE_TRY_COUNT + 1;
                continue;
            }
            if (errno == EAGAIN || errno == ETIMEDOUT) {
                continue;
            }
            // All attempts made, hard failure
            break;
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
            std::list<std::unique_ptr<TTContactsMessage>> messages;
            auto cvStatus = cv_status::no_timeout;
            while (true)
            {
                {
                    std::unique_lock<std::mutex> lock(mQueueMutex);
                    auto waitTimeMs = std::chrono::miliseconds(TTCONTACTS_HEARTBEAT_TIMEOUT_MS);
                    cvStatus = mQueueCondition.wait_for(lock, waitTimeMs, [this]() {
                        return !mQueuedMessages.empty() || mForcedQuit.load();
                    });

                    if (mForcedQuit.load()) {
                        throw std::runtime_error({});
                    }

                    while (!mQueuedMessages.empty()) {
                        messages.push_back(std::move(mQueuedMessages.front()));
                        mQueuedMessages.pop();
                    }
                }

                if (cvStatus == cv_status::timeout) {
                    send(TTChatMessageType::HEARTBEAT, {}, std::chrono::system_clock::now());
                    continue;
                }
                break;
            }

            for (auto &message : messages) {
                int result = -1;
                const char* rawMessage = message.get();
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
                                          rawMessage,
                                          TTCHAT_MESSAGE_MAX_LENGTH,
                                          TTCHAT_MESSAGE_PRIORITY,
                                          &ts);
                    if (result != -1) {
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
