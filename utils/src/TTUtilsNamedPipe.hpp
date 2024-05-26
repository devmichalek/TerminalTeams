#pragma once
#include "TTUtilsSyscall.hpp"
#include <string>
#include <memory>

class TTUtilsNamedPipe {
public:
    explicit TTUtilsNamedPipe(const std::string& path,
        long messageSize,
        std::shared_ptr<TTUtilsSyscall> syscall);
    virtual ~TTUtilsNamedPipe();
    TTUtilsNamedPipe(const TTUtilsNamedPipe&) = delete;
    TTUtilsNamedPipe(TTUtilsNamedPipe&&) = delete;
    TTUtilsNamedPipe& operator=(const TTUtilsNamedPipe&) = delete;
    TTUtilsNamedPipe& operator=(TTUtilsNamedPipe&&) = delete;
    virtual bool alive() const;
    virtual bool create();
    virtual bool open(long attempts = 3, long timeoutMs = 500);
    virtual bool receive(char* message);
    virtual bool send(const char* message);
 protected:
    TTUtilsNamedPipe() = default;
private:
    // IPC shared memory communication
    std::string mNamedPipePath;
    long mMessageSize;
    int mNamedPipeDescriptor;
    std::shared_ptr<TTUtilsSyscall> mSyscall;
};