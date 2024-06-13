#include "TTUtilsNamedPipe.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <thread>
#include <chrono>

TTUtilsNamedPipe::TTUtilsNamedPipe(const std::string& path,
    long messageSize,
    std::shared_ptr<TTUtilsSyscall> syscall) :
        mNamedPipePath(path),
        mMessageSize(messageSize), 
        mNamedPipeDescriptor(-1),
        mSyscall(std::move(syscall)) {
    LOG_INFO("Successfully constructed!");
}

TTUtilsNamedPipe::~TTUtilsNamedPipe() {
    LOG_INFO("Destructing...");
    if (alive()) {
        mSyscall->close(mNamedPipeDescriptor);
        mNamedPipeDescriptor = -1;
    }
    if (!mNamedPipePath.empty()) {
        mSyscall->unlink(mNamedPipePath.c_str());
        mNamedPipePath.clear();
    }
    LOG_INFO("Successfully destructed!");
}

bool TTUtilsNamedPipe::alive() const {
    return mNamedPipeDescriptor != -1;
}

bool TTUtilsNamedPipe::create() {
    LOG_INFO("Creating \"{}\"...", mNamedPipePath);
    if (alive()) {
        LOG_ERROR("Cannot recreate!");
        return false;
    }
    errno = 0;
    if (mSyscall->mkfifo(mNamedPipePath.c_str(), 0666) < 0) {
        LOG_ERROR("Failed to create named pipe \"{}\", errno={}", mNamedPipePath, errno);
        return false;
    }

    mNamedPipeDescriptor = mSyscall->open(mNamedPipePath.c_str(), O_RDONLY);
    if (mNamedPipeDescriptor == -1) {
        LOG_ERROR("Failed to open named pipe \"{}\", errno={}", mNamedPipePath, errno);
        return false;
    }
    LOG_INFO("Successfully created!");
    return true;
}

bool TTUtilsNamedPipe::open(long attempts, long timeoutMs) {
    LOG_INFO("Opening \"{}\"...", mNamedPipePath);
    if (alive()) {
        LOG_ERROR("Cannot reopen!");
        return false;
    }
    const auto path = mNamedPipePath;
    mNamedPipePath.clear();
    errno = 0;
    for (auto attempt = attempts; attempt > 0; --attempt) {
        if ((mNamedPipeDescriptor = mSyscall->open(path.c_str(), O_WRONLY)) != -1) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
    }

    if (mNamedPipeDescriptor == -1) {
        LOG_ERROR("Failed to open named pipe \"{}\", errno={}", path, errno);
        return false;
    }
    LOG_INFO("Successfully opened!");
    return true;
}

bool TTUtilsNamedPipe::receive(char* message) {
    errno = 0;
    if (mSyscall->read(mNamedPipeDescriptor, message, mMessageSize) < 0) {
        LOG_ERROR("Hard failure while receiving message, errno={}", errno);
        return false;
    }
    LOG_INFO("Successfully received message!");
    return true;
}

bool TTUtilsNamedPipe::send(const char* message) {
    errno = 0;
    if (mSyscall->write(mNamedPipeDescriptor, message, mMessageSize) < 0) {
        LOG_ERROR("Hard failure while sending message, errno={}", errno);
        return false;
    }
    LOG_INFO("Successfully sent message!");
    return true;
}
