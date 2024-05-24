#include "TTUtilsMessageQueue.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <thread>

TTUtilsMessageQueue::TTUtilsMessageQueue(std::string name,
    long queueSize,
    long messageSize,
    std::shared_ptr<TTUtilsSyscall> syscall) :
        mName(name),
        mDescriptor(-1),
        mDeleter([](const std::string&){}),
        mQueueSize(queueSize),
        mMessageSize(messageSize),
        mSyscall(std::move(syscall)) {
    TTDiagnosticsLogger::getInstance().info("{} Constructing...", mClassNamePrefix);
}

TTUtilsMessageQueue::~TTUtilsMessageQueue() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    mDeleter(mName);
    mDescriptor = -1;
}

bool TTUtilsMessageQueue::create() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Creating...", mClassNamePrefix);
    if (alive()) {
        logger.error("{} Cannot recreate!", mClassNamePrefix);
        return false;
    }
    struct mq_attr messageQueueAttributes;
    messageQueueAttributes.mq_maxmsg = mQueueSize;
    messageQueueAttributes.mq_msgsize = mMessageSize;
    messageQueueAttributes.mq_flags = 0;
    errno = 0;
    mDescriptor = mSyscall->mq_open(mName.c_str(), O_CREAT | O_RDWR, 0644, &messageQueueAttributes);
    if (mDescriptor == -1) {
        logger.error("{} Failed to create message queue, errno={}", mClassNamePrefix, errno);
        return false;
    }
    mDeleter = [this](const std::string& name){ mSyscall->mq_unlink(name.c_str()); };
    logger.info("{} Successfully created!", mClassNamePrefix);
    return true;
}

bool TTUtilsMessageQueue::open(long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Opening...", mClassNamePrefix);
    if (alive()) {
        logger.error("{} Cannot reopen!", mClassNamePrefix);
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
        logger.error("{} Failed to open message queue, errno={}", mClassNamePrefix, errno);
        return false;
    }
    logger.info("{} Successfully opened!", mClassNamePrefix);
    return true;
}

bool TTUtilsMessageQueue::alive() const {
    return mDescriptor != -1;
}

bool TTUtilsMessageQueue::receive(char* message, long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        for (auto i = attempts; i > 0; --i) {
            errno = 0;
            struct timespec ts;
            if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                logger.error("{} Hard failure while sending message, fetching clock time, errno={}", mClassNamePrefix, errno);
                break;
            }
            ts.tv_sec += timeoutSecs;
            unsigned int priority = 0;
            auto res = mSyscall->mq_timedreceive(mDescriptor, message, mMessageSize, &priority, &ts);
            if (res != -1) {
                logger.error("{} Successfully received message!", mClassNamePrefix);
                result = true;
                break;
            }
            if (errno == EAGAIN) {
                logger.warning("{} Soft failure while receiving message, resource temporarily unavailable, errno={}", mClassNamePrefix, errno);
                continue;
            }
            if (errno == ETIMEDOUT) {
                logger.warning("{} Soft failure while receiving message, timeout, errno={}", mClassNamePrefix, errno);
                continue;
            }
            logger.error("{} Hard failure while receiving message, errno={}", mClassNamePrefix, errno);
            break;
        }
    }
    return result;
}

bool TTUtilsMessageQueue::send(const char* message, long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        for (auto i = attempts; i > 0; --i) {
            errno = 0;
            struct timespec ts;
            if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                logger.error("{} Hard failure while sending message, fetching clock time, errno={}", mClassNamePrefix, errno);
                break;
            }
            ts.tv_sec += timeoutSecs;
            unsigned int priority = 0;
            auto res = mSyscall->mq_timedsend(mDescriptor, message, mMessageSize, priority, &ts);
            if (res != -1) {
                logger.error("{} Successfully send message!", mClassNamePrefix);
                result = true;
                break;
            }
            if (errno == EAGAIN) {
                logger.warning("{} Soft failure while sending message, resource temporarily unavailable, errno={}", mClassNamePrefix, errno);
                continue;
            }
            if (errno == ETIMEDOUT) {
                logger.warning("{} Soft failure while sending message, timeout, errno={}", mClassNamePrefix, errno);
                continue;
            }
            logger.error("{} Hard failure while sending message, errno={}", mClassNamePrefix, errno);
            break;
        }
    }
    return result;
}
