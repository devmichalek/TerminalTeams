#include "TTUtilsNamedPipe.hpp"

TTUtilsNamedPipe::TTUtilsNamedPipe(std::string path,
    long messageSize,
    std::shared_ptr<TTUtilsSyscall> syscall) :
        mNamedPipePath(path),
        mMessageSize(messageSize), 
        mNamedPipeDescriptor(-1) {
    TTDiagnosticsLogger::getInstance().info("{} Constructing...", mClassNamePrefix);
}

TTUtilsNamedPipe::~TTUtilsNamedPipe() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    if (alive()) {
        mSyscall->close(mNamedPipeDescriptor);
        mNamedPipeDescriptor = -1;
    }
    if (!mNamedPipePath.empty()) {
        mSyscall->unlink(mNamedPipePath.c_str());
        mNamedPipePath.clear();
    }
}

bool TTUtilsNamedPipe::alive() const {
    mNamedPipeDescriptor != -1;
}

bool TTUtilsNamedPipe::create() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Creating...", mClassNamePrefix);
    if (alive()) {
        logger.error("{} Cannot recreate!", mClassNamePrefix);
        return false;
    }
    errno = 0;
    if (mSyscall->mkfifo(mNamedPipePath.c_str(), 0666) < 0) {
        logger.error("{} Failed to create named pipe, errno={}", mClassNamePrefix, errno);
        return false;
    }

    mNamedPipeDescriptor = mSyscall->open(mNamedPipePath.c_str(), O_RDONLY);
    if (mNamedPipeDescriptor == -1) {
        logger.error("{} Failed to open named pipe, errno={}", mClassNamePrefix, errno);
        return false;
    }
    logger.info("{} Successfully created!", mClassNamePrefix);
    return true;
}

bool TTUtilsNamedPipe::open(long attempts, long timeoutMs) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Opening...", mClassNamePrefix);
    if (alive()) {
        logger.error("{} Cannot reopen!", mClassNamePrefix);
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
        logger.error("{} Failed to open named pipe, errno={}", mClassNamePrefix, errno);
        return false;
    }
    logger.info("{} Successfully opened!", mClassNamePrefix);
    return true;
}

bool TTUtilsNamedPipe::receive(char* message) {
    errno = 0;
    bool result = mSyscall->read(mNamedPipeDescriptor, message, messageSize) < 0;
    if (!result) {
        logger.error("{} Hard failure while receiving message, errno={}", mClassNamePrefix, errno);
    }
    logger.info("{} Successfully received message!", mClassNamePrefix);
    return result;
}

bool TTUtilsNamedPipe::send(const char* message) {
    errno = 0;
    bool result = mSyscall->write(mNamedPipeDescriptor, message, messageSize) < 0;
    if (!result) {
        logger.error("{} Hard failure while receiving message, errno={}", mClassNamePrefix, errno);
    }
    logger.info("{} Successfully received message!", mClassNamePrefix);
    return result;
}
