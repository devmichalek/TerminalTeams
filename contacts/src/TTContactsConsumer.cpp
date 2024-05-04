#include "TTContactsConsumer.hpp"
#include <cstring>
#include <iostream>
#include <thread>

TTContactsConsumer::TTContactsConsumer(const std::string& sharedMemoryName,
    const std::string& dataConsumedSemName,
    const std::string& dataProducedSemName,
    std::shared_ptr<TTContactsSyscall> syscall) : 
        mSharedMemoryName(sharedMemoryName),
        mDataConsumedSemName(dataConsumedSemName),
        mDataProducedSemName(dataProducedSemName),
        mSyscall(std::move(syscall)),
        mSharedMessage(nullptr),
        mDataProducedSemaphore(nullptr),
        mDataConsumedSemaphore(nullptr),
        mAlive(false),
        mLogger(TTDiagnosticsLogger::getInstance()) {
    mLogger.info("{} Constructing...", mClassNamePrefix);
}

bool TTContactsConsumer::init(long attempts, long timeoutMs) {
    mLogger.info("{} Initializing...", mClassNamePrefix);
    if (mAlive) {
        mLogger.error("{} Cannot reinitialize!", mClassNamePrefix);
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
        mLogger.error("{} Failed to open data produced semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }

    errno = 0;
    if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), 0)) == SEM_FAILED) {
        mLogger.error("{} Failed to open data consumed semaphore, errno={}", mClassNamePrefix, errno);
        return false;
    }

    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        mLogger.error("{} Failed to open shared object, errno={}", mClassNamePrefix, errno);
        return false;
    }

    void* rawPointer = mSyscall->mmap(nullptr, sizeof(TTContactsMessage), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    mSharedMessage = new(rawPointer) TTContactsMessage;
    if (!mSharedMessage) {
        mLogger.error("{} Failed to allocate shared message!", mClassNamePrefix);
        return false;
    }

    mLogger.info("{} Successfully initialized!", mClassNamePrefix);
    mAlive = true;
    return true;
}

std::unique_ptr<TTContactsMessage> TTContactsConsumer::get(long attempts, long timeoutMs) {
    const auto timeoutSecs = timeoutMs / 1000;
    std::unique_ptr<TTContactsMessage> result = nullptr;
    if (mAlive) {
        do {
            {
                int res = -1;
                for (auto attempt = attempts; attempt > 0; --attempt) {
                    mLogger.info("{} Waiting for the other process to produce the data, attempt={}", mClassNamePrefix, (attempts - attempt));
                    struct timespec ts;
                    if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        res = -1;
                        mLogger.error("{} Hard failure at fetching clock time!", mClassNamePrefix);
                        break;
                    }
                    ts.tv_sec += timeoutSecs;
                    res = mSyscall->sem_timedwait(mDataProducedSemaphore, &ts);
                    if (res != -1) {
                        mLogger.info("{} Fetched other process data", mClassNamePrefix);
                        break;
                    } else if (errno == EINTR) {
                        mLogger.warning("{} Soft failure, interrupted function call!", mClassNamePrefix);
                        continue;
                    }
                }
                if (res == -1) {
                    break;
                }
            }

            result = std::make_unique<TTContactsMessage>();
            memcpy(result.get(), mSharedMessage, sizeof(TTContactsMessage));
            if (mSyscall->sem_post(mDataConsumedSemaphore) == -1) {
                mLogger.error("{} Failed to notify other process!", mClassNamePrefix);
                result = nullptr;
            } else {
                mLogger.info("{} Successfully notified other process", mClassNamePrefix);
            }
        } while (false);
    }
    
    mAlive = (result != nullptr);
    mLogger.info("{} Summary of the fetched message, alive={}", mClassNamePrefix, mAlive);
    return result;
}

bool TTContactsConsumer::alive() const {
    return mAlive;
}