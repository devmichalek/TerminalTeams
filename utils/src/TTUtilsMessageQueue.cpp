#include "TTUtilsMessageQueue.hpp"
#include <thread>

TTUtilsMessageQueue::TTUtilsMessageQueue(std::string name, long queueSize, long messageSize) :
        mName(name),
        mDescriptor(-1),
        mDeleter([](const std::string&){}),
        mQueueSize(queueSize),
        mMessageSize(messageSize) {
    mBuffer.resize(mMessageSize);
}

TTUtilsMessageQueue::~TTUtilsMessageQueue() {
    mDeleter(mName);
}

bool TTUtilsMessageQueue::open(long attempts, long timeoutMs) {
    struct mq_attr messageQueueAttributes;
    messageQueueAttributes.mq_maxmsg = mQueueSize;
    messageQueueAttributes.mq_msgsize = mMessageSize;
    messageQueueAttributes.mq_flags = 0;
    errno = 0;
    for (auto attempt = attempts; attempt > 0; --attempt) {
        mDescriptor = mq_open(mName.c_str(), O_RDWR, 0644, &messageQueueAttributes);
        if (mDescriptor != -1) {
            break; // Success
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }
    if (mDescriptor == -1) {
        // throw std::runtime_error(classNamePrefix + "Failed to open message queue, errno=" + std::to_string(errno));
        return false;
    }

    return true;
}

bool TTUtilsMessageQueue::create() {
    struct mq_attr messageQueueAttributes;
    messageQueueAttributes.mq_maxmsg = mQueueSize;
    messageQueueAttributes.mq_msgsize = mMessageSize;
    messageQueueAttributes.mq_flags = 0;
    errno = 0;
    mDescriptor = mq_open(mName.c_str(), O_CREAT | O_RDWR, 0644, &messageQueueAttributes);
    if (mDescriptor == -1) {
        // throw std::runtime_error(classNamePrefix + "Failed to create message queue, errno=" + std::to_string(errno));
        return false;
    }
    mDeleter = [](const std::string& name){ mq_unlink(name.c_str()); };
    return true;
}

bool TTUtilsMessageQueue::alive() const {
    return mDescriptor != -1;
}

bool TTUtilsMessageQueue::receive(char* message, long attempts, long timeoutMs) {
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        for (auto i = attempts; i > 0; --i) {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                // Hard failure
                break;
            }
            ts.tv_sec += timeoutSecs;
            errno = 0;
            unsigned int priority = 0;
            auto res = mq_timedreceive(mDescriptor, message, mMessageSize, &priority, &ts);
            if (res != -1) {
                result = true;
                break;
            }
            if (errno == EAGAIN) {
                continue;
            }
            if (errno == ETIMEDOUT) {
                continue;
            }
            // Hard failure
            break;
        }
    }
    return result;
}

bool TTUtilsMessageQueue::send(const char* message, long attempts, long timeoutMs) {
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        for (auto i = attempts; i > 0; --i) {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                // Hard failure
                break;
            }
            ts.tv_sec += timeoutSecs;
            errno = 0;
            unsigned int priority = 0;
            auto res = mq_timedsend(mDescriptor, message, mMessageSize, priority, &ts);
            if (res != -1) {
                result = true;
                break;
            }
            if (errno == EAGAIN) {
                continue;
            }
            if (errno == ETIMEDOUT) {
                continue;
            }
            // Hard failure
            break;
        }
    }
    return result;
}
