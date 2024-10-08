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
    LOG_INFO("Successfully constructed!");
}

TTUtilsSharedMem::~TTUtilsSharedMem() {
    LOG_INFO("Destructing...");
    destroy();
    LOG_INFO("Successfully destructed!");
}

bool TTUtilsSharedMem::create() {
    LOG_INFO("Creating \"{}\", \"{}\" and \"{}\"...", mSharedMemoryName, mDataProducedSemName, mDataConsumedSemName);
    if (alive()) {
        LOG_ERROR("Cannot recreate!");
        return false;
    }

    errno = 0;
    const int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        LOG_ERROR("Failed to create shared object \"{}\", errno={}", mSharedMemoryName, errno);
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
        LOG_ERROR("Failed to create data produced semaphore \"{}\", errno={}", mDataProducedSemName, errno);
        return false;
    }
    mDataProducedSemCreated = true;

    errno = 0;
    if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), O_CREAT, 0644, 0)) == SEM_FAILED) {
        LOG_ERROR("Failed to create data consumed semaphore \"{}\", errno={}", mDataConsumedSemName, errno);
        return false;
    }
    mDataConsumedSemCreated = true;

    LOG_INFO("Successfully created!");
    mAlive = true;
    return true;
}

bool TTUtilsSharedMem::open(long attempts, long timeoutMs) {
    LOG_INFO("Opening \"{}\", \"{}\" and \"{}\"...", mSharedMemoryName, mDataProducedSemName, mDataConsumedSemName);
    if (alive()) {
        LOG_ERROR("Cannot reopen!");
        return false;
    }

    int dataProducedSemErrno = 0;
    int dataConsumedSemErrno = 0;
    int sharedMemErrno = 0;
    int fileDescriptor = 0;
    for (auto attempt = attempts; attempt > 0; --attempt) {
        
        if (!mDataProducedSemaphore) {
            errno = 0;
            mDataProducedSemaphore = mSyscall->sem_open(mDataProducedSemName.c_str(), 0);
            dataProducedSemErrno = errno;
        }
        if (!mDataConsumedSemaphore) {
            errno = 0;
            mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), 0);
            dataConsumedSemErrno = errno;
        }
        if (fileDescriptor <= 0) {
            errno = 0;
            fileDescriptor = mSyscall->shm_open(mSharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
            sharedMemErrno = errno;
        }

        if (mDataProducedSemaphore && mDataConsumedSemaphore && fileDescriptor > 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }

    if (!mDataProducedSemaphore) {
        LOG_ERROR("Failed to open data produced semaphore \"{}\", errno={}", mDataProducedSemName, dataProducedSemErrno);
        return false;
    }

    if (!mDataConsumedSemaphore) {
        LOG_ERROR("Failed to open data consumed semaphore \"{}\", errno={}", mDataConsumedSemName, dataConsumedSemErrno);
        return false;
    }

    errno = 0;
    if (fileDescriptor < 0) {
        LOG_ERROR("Failed to open shared object \"{}\", errno={}", mSharedMemoryName, sharedMemErrno);
        return false;
    }

    mSharedMessage = mSyscall->mmap(nullptr, mSharedMessageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
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

bool TTUtilsSharedMem::destroy() {
    bool result = true;
    if (mSharedMemoryCreated) {
        mSharedMemoryCreated = false;
        const auto code = mSyscall->shm_unlink(mSharedMemoryName.c_str());
        result &= (code == 0);
        if (code != 0) {
            LOG_ERROR("Failed to unlink shared memory! Error code: {}", code);
        }
    }
    if (mDataConsumedSemCreated) {
        mDataConsumedSemCreated = false;
        const auto code = mSyscall->sem_unlink(mDataConsumedSemName.c_str());
        result &= (code == 0);
        if (code != 0) {
            LOG_ERROR("Failed to unlink semaphore! Error code: {}", code);
        }
    }
    if (mDataProducedSemCreated) {
        mDataProducedSemCreated = false;
        const auto code = mSyscall->sem_unlink(mDataProducedSemName.c_str());
        result &= (code == 0);
        if (code != 0) {
            LOG_ERROR("Failed to unlink semaphore! Error code: {}", code);
        }
    }
    return result;
}