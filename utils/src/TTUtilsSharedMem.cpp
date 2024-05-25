#include "TTUtilsSharedMem.hpp"
#include <cstring>
#include <iostream>
#include <thread>

TTUtilsSharedMem::TTUtilsSharedMem(const std::string& sharedMemoryName,
    const std::string& dataConsumedSemName,
    const std::string& dataProducedSemName,
    size_t sharedMessageSize,
    std::shared_ptr<TTUtilsSyscall> syscall) : 
        mSharedMemoryName(sharedMemoryName),
        mDataConsumedSemName(dataConsumedSemName),
        mDataProducedSemName(dataProducedSemName),
        mSyscall(std::move(syscall)),
        mSharedMessage(nullptr),
        mSharedMessageSize(sharedMessageSize),
        mDataProducedSemaphore(nullptr),
        mDataConsumedSemaphore(nullptr),
        mAlive(false),
        mSharedMemoryCreated(false),
        mDataConsumedSemCreated(false),
        mDataProducedSemCreated(false) {
    TTDiagnosticsLogger::getInstance().info("{} Constructing...", mClassNamePrefix);
}

TTUtilsSharedMem::~TTUtilsSharedMem() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    destroy();
}

bool TTUtilsSharedMem::create() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Creating...", mClassNamePrefix);
    if (alive()) {
        logger.error("{} Cannot recreate!", mClassNamePrefix);
        return false;
    }

    errno = 0;
    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        logger.error("{} Failed to create shared object, errno={}", mClassNamePrefix, errno);
        return false;
    }
    mSharedMemoryCreated = true;

    errno = 0;
    if (mSyscall->ftruncate(fd, mSharedMessageSize) == -1) {
        logger.error("{} Failed to truncate shared object, errno={}", mClassNamePrefix, errno);
        return false;
    }

    mSharedMessage = mSyscall->mmap(nullptr, mSharedMessageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!mSharedMessage) {
        logger.error("{} Failed to mmap shared message!", mClassNamePrefix);
        return false;
    }

    errno = 0;
    if ((mDataProducedSemaphore = mSyscall->sem_open(mDataProducedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
        logger.error("{} Failed to create data produced semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }
    mDataProducedSemCreated = true;

    errno = 0;
    if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
        logger.error("{} Failed to create data consumed semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }
    mDataConsumedSemCreated = true;

    logger.info("{} Successfully created!", mClassNamePrefix);
    mAlive = true;
    return true;
}

bool TTUtilsSharedMem::open(long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Opening...", mClassNamePrefix);
    if (alive()) {
        logger.error("{} Cannot reopen!", mClassNamePrefix);
        return false;
    }

    errno = 0;
    for (auto attempt = attempts; attempt > 0; --attempt) {
        
        if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), 0)) != SEM_FAILED) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }

    if (mDataConsumedSemaphore == SEM_FAILED) {
        logger.error("{} Failed to open data consumed semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }

    errno = 0;
    if ((mDataProducedSemaphore = mSyscall->sem_open(mDataProducedSemName.c_str(), 0)) == SEM_FAILED) {
        logger.error("{} Failed to open data produced semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }

    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        logger.error("{} Failed to open shared object, errno={}", mClassNamePrefix, errno);
        return false;
    }

    mSharedMessage = mSyscall->mmap(nullptr, mSharedMessageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!mSharedMessage) {
        logger.error("{} Failed to mmap shared message!", mClassNamePrefix);
        return false;
    }

    logger.info("{} Successfully opened!", mClassNamePrefix);
    mAlive = true;
    return true;
}

bool TTUtilsSharedMem::receive(void* message, long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        do {
            int code = -1;
            for (auto attempt = attempts; attempt > 0; --attempt) {
                logger.info("{} Waiting for the other process to produce the data, attempt={}", mClassNamePrefix, (attempts - attempt));
                struct timespec ts;
                if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                    code = -1;
                    logger.error("{} Hard failure at fetching clock time before data was received!", mClassNamePrefix);
                    break;
                }
                ts.tv_sec += timeoutSecs;
                code = mSyscall->sem_timedwait(mDataProducedSemaphore, &ts);
                if (code != -1) {
                    logger.info("{} Received other process data", mClassNamePrefix);
                    break;
                } else if (errno == EINTR) {
                    logger.warning("{} Soft failure, interrupted function call before data was received!", mClassNamePrefix);
                    continue;
                }
            }
            if (code == -1) {
                break;
            }
            memcpy(message, mSharedMessage, mSharedMessageSize);
            if (mSyscall->sem_post(mDataConsumedSemaphore) == -1) {
                logger.error("{} Failed to notify other process about received data!", mClassNamePrefix);
                break;
            } else {
                result = true;
                logger.info("{} Successfully notified other process about received data", mClassNamePrefix);
            }
        } while (false);
    }
    mAlive = result;
    logger.info("{} Finished receiving data, alive={}", mClassNamePrefix, mAlive);
    return result;
}

bool TTUtilsSharedMem::send(const void* message, long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        do {
            memcpy(mSharedMessage, message, mSharedMessageSize);
            if (mSyscall->sem_post(mDataProducedSemaphore) == -1) {
                logger.error("{} Failed to notify other process about sent data!", mClassNamePrefix);
                break;
            } else {
                logger.info("{} Successfully notified other process about sent data", mClassNamePrefix);
            }

            int code = 0;
            for (auto attempt = attempts; attempt > 0; --attempt) {
                logger.info("{} Waiting for the other process to consume the data, attempt={}", mClassNamePrefix, (attempts - attempt));
                struct timespec ts;
                if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                    code = -1;
                    logger.error("{} Hard failure at fetching clock time after data was sent!", mClassNamePrefix);
                    break;
                }
                ts.tv_sec += timeoutSecs;
                code = mSyscall->sem_timedwait(mDataConsumedSemaphore, &ts);
                if (code != -1) {
                    logger.info("{} Sent data to other process", mClassNamePrefix);
                    result = true;
                    break;
                } else if (errno == EINTR) {
                    logger.warning("{} Soft failure, interrupted function call after data was sent!", mClassNamePrefix);
                    continue;
                }
            }
        } while (false);
    }
    mAlive = result;
    logger.info("{} Finished sending data, alive={}", mClassNamePrefix, mAlive);
    return result;
}

bool TTUtilsSharedMem::alive() const {
    return mAlive;
}

void TTUtilsSharedMem::destroy() {
    if (mSharedMemoryCreated) {
        mSyscall->shm_unlink(mSharedMemoryName.c_str());
        mSharedMemoryCreated = false;
    }
    if (mDataConsumedSemCreated) {
        mSyscall->sem_unlink(mDataConsumedSemName.c_str());
        mDataConsumedSemCreated = false;
    }
    if (mDataProducedSemCreated) {
        mSyscall->sem_unlink(mDataProducedSemName.c_str());
        mDataProducedSemCreated = false;
    }
}