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
        mAlive(false) {}

bool TTContactsConsumer::init(long attempts, long timeoutMs) {
    const std::string classNamePrefix = "TTContactsConsumer: ";
    if (mAlive) {
        std::cerr << classNamePrefix << "Cannot reinitialize!" << std::endl;
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
        std::cerr << classNamePrefix << "Failed to open data produced semaphore, errno=" << errno << std::endl;
        return false;
    }

    errno = 0;
    if ((mDataConsumedSemaphore = mSyscall->sem_open(mDataConsumedSemName.c_str(), 0)) == SEM_FAILED) {
        std::cerr << classNamePrefix << "Failed to open data consumed semaphore, errno=" << errno << std::endl;
        return false;
    }

    int fd = mSyscall->shm_open(mSharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        std::cerr << classNamePrefix << "Failed to open shared object, errno=" << errno << std::endl;
        return false;
    }

    void* rawPointer = mSyscall->mmap(nullptr, sizeof(TTContactsMessage), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    mSharedMessage = new(rawPointer) TTContactsMessage;
    if (!mSharedMessage) {
        std::cerr << classNamePrefix << "Failed to allocate shared message!" << std::endl;
        return false;
    }

    mAlive = true;
    return true;
}

std::unique_ptr<TTContactsMessage> TTContactsConsumer::get(long attempts, long timeoutMs) {
    const auto timeoutSecs = timeoutMs / 1000;
    std::unique_ptr<TTContactsMessage> result = nullptr;
    if (mAlive) {
        do {
            // Wait for the other process to produce the data
            {
                int res = -1;
                for (auto attempt = attempts; attempt > 0; --attempt) {
                    struct timespec ts;
                    if (mSyscall->clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        res = -1;
                        break; // Hard failure
                    }
                    ts.tv_sec += timeoutSecs;
                    res = mSyscall->sem_timedwait(mDataProducedSemaphore, &ts);
                    if (res != -1) {
                        break; // Success
                    } else if (errno == EINTR) {
                        continue; // Soft failure
                    }
                }
                if (res == -1) {
                    break;
                }
            }

            result = std::make_unique<TTContactsMessage>();
            memcpy(result.get(), mSharedMessage, sizeof(TTContactsMessage));
            if (mSyscall->sem_post(mDataConsumedSemaphore) == -1) {
                result = nullptr;
            }
        } while (false);
    }
    mAlive = (result != nullptr);
    return result;
}

bool TTContactsConsumer::alive() const {
    return mAlive;
}