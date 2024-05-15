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
    virtual ~TTUtilsSharedMem() {}
    TTUtilsSharedMem(const TTUtilsSharedMem&) = delete;
    TTUtilsSharedMem(TTUtilsSharedMem&&) = delete;
    TTUtilsSharedMem& operator=(const TTUtilsSharedMem&) = delete;
    TTUtilsSharedMem& operator=(TTUtilsSharedMem&&) = delete;
    // Initialize system objects
    virtual bool init(long attempts = 5, long timeoutMs = 1000);
    // Receive with timeout
    virtual bool receive(void* message, long attempts = 2, long timeoutMs = 2000);
    // Check if object is functioning properly
    virtual bool alive() const;
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
    // Logger
    inline static const std::string mClassNamePrefix = "TTUtilsSharedMem:";
};