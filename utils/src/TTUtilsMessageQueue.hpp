#pragma once
#include <string>
#include <memory>
#include <functional>
#include <mqueue.h>

class TTUtilsMessageQueue {
 public:
    explicit TTUtilsMessageQueue(std::string name, long queueSize, long messageSize);
    virtual ~TTUtilsMessageQueue();
    TTUtilsMessageQueue(const TTUtilsMessageQueue&) = delete;
    TTUtilsMessageQueue(TTUtilsMessageQueue&&) = delete;
    TTUtilsMessageQueue& operator=(const TTUtilsMessageQueue&) = delete;
    TTUtilsMessageQueue& operator=(TTUtilsMessageQueue&&) = delete;
    virtual bool open(long attempts = 5, long timeoutMs = 1000);
    virtual bool create();
    virtual bool alive() const;
    virtual bool receive(char* message, long attempts = 3, long timeoutMs = 1000);
    virtual bool send(const char* message, long attempts = 3, long timeoutMs = 1000);
 private:
    std::string mName;
    mqd_t mDescriptor;
    std::function<void(const std::string&)> mDeleter;
    long mQueueSize;
    long mMessageSize;
    std::vector<char> mBuffer;
    // Logger
    inline static const std::string mClassNamePrefix = "TTUtilsMessageQueue:";
};
