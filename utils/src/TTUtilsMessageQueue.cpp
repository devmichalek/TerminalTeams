#include "TTUtilsMessageQueue.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <thread>

TTUtilsMessageQueue::TTUtilsMessageQueue(std::string name,
    long queueSize,
    long messageSize,
    std::shared_ptr<TTUtilsSyscall> syscall) :
        mName(name),
        mDescriptor(-1),
        mDeleter({}),
        mQueueSize(queueSize),
        mMessageSize(messageSize),
        mSyscall(std::move(syscall)) {
    LOG_INFO("Successfully constructed!");
}

TTUtilsMessageQueue::~TTUtilsMessageQueue() {
    LOG_INFO("Destructing...");
    if (mDeleter) {
        mDeleter(mName);
    }
    mDescriptor = -1;
    LOG_INFO("Successfully destructed!");
}

bool TTUtilsMessageQueue::create() {
    LOG_INFO("Creating...");
    if (alive()) {
        LOG_ERROR("Cannot recreate!");
        return false;
    }
    struct mq_attr messageQueueAttributes;
    messageQueueAttributes.mq_maxmsg = mQueueSize;
    messageQueueAttributes.mq_msgsize = mMessageSize;
    messageQueueAttributes.mq_flags = 0;
    errno = 0;
    mDescriptor = mSyscall->mq_open(mName.c_str(), O_CREAT | O_RDWR, 0644, &messageQueueAttributes);
    if (mDescriptor == -1) {
        LOG_ERROR("Failed to create message queue, errno={}", errno);
        return false;
    }
    mDeleter = [this](const std::string& name){ mSyscall->mq_unlink(name.c_str()); };
    LOG_INFO("Successfully created!");
    return true;
}

bool TTUtilsMessageQueue::open(long attempts, long timeoutMs) {
    LOG_INFO("Opening...");
    if (alive()) {
        LOG_ERROR("Cannot reopen!");
        return false;
    }
    struct mq_attr messageQueueAttributes;
    messageQueueAttributes.mq_maxmsg = mQueueSize;
    messageQueueAttributes.mq_msgsize = mMessageSize;
    messageQueueAttributes.mq_flags = 0;
    errno = 0;
    for (auto attempt = attempts; attempt > 0; --attempt) {
        mDescriptor = mSyscall->mq_open(mName.c_str(), O_RDWR, 0644, &messageQueueAttributes);
        if (mDescriptor != -1) {
            break; // Success
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }
    if (mDescriptor == -1) {
        LOG_ERROR("Failed to open message queue, errno={}", errno);
        return false;
    }
    LOG_INFO("Successfully opened!");
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
            errno = 0;
            struct timespec ts;
            if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                LOG_ERROR("Hard failure while sending message, fetching clock time, errno={}", errno);
                break;
            }
            ts.tv_sec += timeoutSecs;
            unsigned int priority = 0;
            auto res = mSyscall->mq_timedreceive(mDescriptor, message, mMessageSize, &priority, &ts);
            if (res != -1) {
                LOG_INFO("Successfully received message!");
                result = true;
                break;
            }
            if (errno == EAGAIN) {
                LOG_WARNING("Soft failure while receiving message, resource temporarily unavailable, errno={}", errno);
                continue;
            }
            if (errno == ETIMEDOUT) {
                LOG_WARNING("Soft failure while receiving message, timeout, errno={}", errno);
                continue;
            }
            LOG_ERROR("Hard failure while receiving message, errno={}", errno);
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
            errno = 0;
            struct timespec ts;
            if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                LOG_ERROR("Hard failure while sending message, fetching clock time, errno={}", errno);
                break;
            }
            ts.tv_sec += timeoutSecs;
            unsigned int priority = 0;
            auto res = mSyscall->mq_timedsend(mDescriptor, message, mMessageSize, priority, &ts);
            if (res != -1) {
                LOG_INFO("Successfully send message!");
                result = true;
                break;
            }
            if (errno == EAGAIN) {
                LOG_WARNING("Soft failure while sending message, resource temporarily unavailable, errno={}", errno);
                continue;
            }
            if (errno == ETIMEDOUT) {
                LOG_WARNING("Soft failure while sending message, timeout, errno={}", errno);
                continue;
            }
            LOG_ERROR("Hard failure while sending message, errno={}", errno);
            break;
        }
    }
    return result;
}
