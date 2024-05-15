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
        mAlive(false) {
    TTDiagnosticsLogger::getInstance().info("{} Constructing...", mClassNamePrefix);
}

bool TTUtilsSharedMem::init(long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Initializing...", mClassNamePrefix);
    if (mAlive) {
        logger.error("{} Cannot reinitialize!", mClassNamePrefix);
        return false;
    }

    errno = 0;
    for (auto attempt = attempts; attempt > 0; --attempt) {
        
        if ((mDataProducedSemaphore = mSyscall->sem_open(mDataProducedSemName.c_str(), 0)) != SEM_FAILED) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }

    if (mDataProducedSemaphore == SEM_FAILED) {
        logger.error("{} Failed to open data produced semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }

    errno = 0;
    if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), 0)) == SEM_FAILED) {
        logger.error("{} Failed to open data consumed semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }

    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        logger.error("{} Failed to open shared object, errno={}", mClassNamePrefix, errno);
        return false;
    }

    mSharedMessage = mSyscall->mmap(nullptr, mSharedMessageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!mSharedMessage) {
        logger.error("{} Failed to allocate shared message!", mClassNamePrefix);
        return false;
    }

    logger.info("{} Successfully initialized!", mClassNamePrefix);
    mAlive = true;
    return true;
}

bool TTUtilsSharedMem::receive(void* message, long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (mAlive) {
        do {
            {
                int res = -1;
                for (auto attempt = attempts; attempt > 0; --attempt) {
                    logger.info("{} Waiting for the other process to produce the data, attempt={}", mClassNamePrefix, (attempts - attempt));
                    struct timespec ts;
                    if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        res = -1;
                        logger.error("{} Hard failure at fetching clock time!", mClassNamePrefix);
                        break;
                    }
                    ts.tv_sec += timeoutSecs;
                    res = mSyscall->sem_timedwait(mDataProducedSemaphore, &ts);
                    if (res != -1) {
                        logger.info("{} Fetched other process data", mClassNamePrefix);
                        break;
                    } else if (errno == EINTR) {
                        logger.warning("{} Soft failure, interrupted function call!", mClassNamePrefix);
                        continue;
                    }
                }
                if (res == -1) {
                    break;
                }
            }

            result = true;
            memcpy(message, mSharedMessage, mSharedMessageSize);
            if (mSyscall->sem_post(mDataConsumedSemaphore) == -1) {
                logger.error("{} Failed to notify other process!", mClassNamePrefix);
                result = false;
            } else {
                logger.info("{} Successfully notified other process", mClassNamePrefix);
            }
        } while (false);
    }
    
    mAlive = result;
    logger.info("{} Summary of the fetched message, alive={}", mClassNamePrefix, mAlive);
    return result;
}

bool TTUtilsSharedMem::alive() const {
    return mAlive;
}