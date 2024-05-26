#pragma once
#include "TTUtilsSyscall.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <memory>
#include <string>

class TTUtilsSharedMem {
public:
    explicit TTUtilsSharedMem(const std::string& sharedMemoryName,
        const std::string& dataConsumedSemName,
        const std::string& dataProducedSemName,
        size_t sharedMessageSize,
        std::shared_ptr<TTUtilsSyscall> syscall);
    virtual ~TTUtilsSharedMem();
    TTUtilsSharedMem(const TTUtilsSharedMem&) = delete;
    TTUtilsSharedMem(TTUtilsSharedMem&&) = delete;
    TTUtilsSharedMem& operator=(const TTUtilsSharedMem&) = delete;
    TTUtilsSharedMem& operator=(TTUtilsSharedMem&&) = delete;
    virtual bool create();
    virtual bool open(long attempts = 5, long timeoutMs = 1000);
    virtual bool receive(void* message, long attempts = 3, long timeoutMs = 1000);
    virtual bool send(const void* message, long attempts = 3, long timeoutMs = 1000);
    virtual bool alive() const;
    virtual void destroy();
protected:
    TTUtilsSharedMem() = default;
private:
    // System objects names
    std::string mSharedMemoryName;
    std::string mDataConsumedSemName;
    std::string mDataProducedSemName;
    // IPC shared memory communication
    std::shared_ptr<TTUtilsSyscall> mSyscall;
    void* mSharedMessage;
    size_t mSharedMessageSize;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Flags
    bool mAlive;
    bool mSharedMemoryCreated;
    bool mDataConsumedSemCreated;
    bool mDataProducedSemCreated;
};