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
    LOG_INFO("Constructing...");
}

TTUtilsSharedMem::~TTUtilsSharedMem() {
    LOG_INFO("Destructing...");
    destroy();
}

bool TTUtilsSharedMem::create() {
    LOG_INFO("Creating...");
    if (alive()) {
        LOG_ERROR("Cannot recreate!");
        return false;
    }

    errno = 0;
    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        LOG_ERROR("Failed to create shared object, errno={}", errno);
        return false;
    }
    mSharedMemoryCreated = true;

    errno = 0;
    if (mSyscall->ftruncate(fd, mSharedMessageSize) == -1) {
        LOG_ERROR("Failed to truncate shared object, errno={}", errno);
        return false;
    }

    mSharedMessage = mSyscall->mmap(nullptr, mSharedMessageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!mSharedMessage) {
        LOG_ERROR("Failed to mmap shared message!");
        return false;
    }

    errno = 0;
    if ((mDataProducedSemaphore = mSyscall->sem_open(mDataProducedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
        LOG_ERROR("Failed to create data produced semaphore, errno={}", errno);
        return false;
    }
    mDataProducedSemCreated = true;

    errno = 0;
    if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
        LOG_ERROR("Failed to create data consumed semaphore, errno={}", errno);
        return false;
    }
    mDataConsumedSemCreated = true;

    LOG_INFO("Successfully created!");
    mAlive = true;
    return true;
}

bool TTUtilsSharedMem::open(long attempts, long timeoutMs) {
    LOG_INFO("Opening...");
    if (alive()) {
        LOG_ERROR("Cannot reopen!");
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
        LOG_ERROR("Failed to open data consumed semaphore, errno={}", errno);
        return false;
    }

    errno = 0;
    if ((mDataProducedSemaphore = mSyscall->sem_open(mDataProducedSemName.c_str(), 0)) == SEM_FAILED) {
        LOG_ERROR("Failed to open data produced semaphore, errno={}", errno);
        return false;
    }

    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        LOG_ERROR("Failed to open shared object, errno={}", errno);
        return false;
    }

    mSharedMessage = mSyscall->mmap(nullptr, mSharedMessageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!mSharedMessage) {
        LOG_ERROR("Failed to mmap shared message!");
        return false;
    }

    LOG_INFO("Successfully opened!");
    mAlive = true;
    return true;
}

bool TTUtilsSharedMem::receive(void* message, long attempts, long timeoutMs) {
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        do {
            int code = -1;
            for (auto attempt = attempts; attempt > 0; --attempt) {
                LOG_INFO("Waiting for the other process to produce the data, attempt={}", (attempts - attempt));
                struct timespec ts;
                if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                    code = -1;
                    LOG_ERROR("Hard failure at fetching clock time before data was received!");
                    break;
                }
                ts.tv_sec += timeoutSecs;
                code = mSyscall->sem_timedwait(mDataProducedSemaphore, &ts);
                if (code != -1) {
                    LOG_INFO("Received other process data");
                    break;
                } else if (errno == EINTR) {
                    LOG_WARNING("Soft failure, interrupted function call before data was received!");
                    continue;
                }
            }
            if (code == -1) {
                break;
            }
            memcpy(message, mSharedMessage, mSharedMessageSize);
            if (mSyscall->sem_post(mDataConsumedSemaphore) == -1) {
                LOG_ERROR("Failed to notify other process about received data!");
                break;
            } else {
                result = true;
                LOG_INFO("Successfully notified other process about received data");
            }
        } while (false);
    }
    mAlive = result;
    LOG_INFO("Finished receiving data, alive={}", mAlive);
    return result;
}

bool TTUtilsSharedMem::send(const void* message, long attempts, long timeoutMs) {
    const auto timeoutSecs = timeoutMs / 1000;
    bool result = false;
    if (alive()) {
        do {
            memcpy(mSharedMessage, message, mSharedMessageSize);
            if (mSyscall->sem_post(mDataProducedSemaphore) == -1) {
                LOG_ERROR("Failed to notify other process about sent data!");
                break;
            } else {
                LOG_INFO("Successfully notified other process about sent data");
            }

            int code = 0;
            for (auto attempt = attempts; attempt > 0; --attempt) {
                LOG_INFO("Waiting for the other process to consume the data, attempt={}", (attempts - attempt));
                struct timespec ts;
                if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                    code = -1;
                    LOG_ERROR("Hard failure at fetching clock time after data was sent!");
                    break;
                }
                ts.tv_sec += timeoutSecs;
                code = mSyscall->sem_timedwait(mDataConsumedSemaphore, &ts);
                if (code != -1) {
                    LOG_INFO("Sent data to other process");
                    result = true;
                    break;
                } else if (errno == EINTR) {
                    LOG_WARNING("Soft failure, interrupted function call after data was sent!");
                    continue;
                }
            }
        } while (false);
    }
    mAlive = result;
    LOG_INFO("Finished sending data, alive={}", mAlive);
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