#pragma once
#include <gmock/gmock.h>
#include <list>
#include "TTUtilsInputStream.hpp"

class TTUtilsInputStreamMock : public TTUtilsInputStream {
public:
    TTUtilsInputStreamMock() = default;
    virtual ~TTUtilsInputStreamMock() {}
    TTUtilsInputStreamMock(const TTUtilsInputStreamMock&) = default;
    TTUtilsInputStreamMock(TTUtilsInputStreamMock&&) = default;
    TTUtilsInputStreamMock& operator=(const TTUtilsInputStreamMock&) = default;
    TTUtilsInputStreamMock& operator=(TTUtilsInputStreamMock&&) = default;

    virtual TTUtilsInputStream& readline(std::string& output) {
        std::unique_lock<std::mutex> lock(mInputMutex);
        mInputCondition.wait(lock, [this]() {return !mInput.empty();});
        output = mInput.back();
        mInput.pop_back();
        return *this;
    }

    virtual void input(const std::string& data) {
        {
            std::scoped_lock lock(mInputMutex);
            mInput.push_front(data);
        }
        mInputCondition.notify_one();
    }

    virtual void clear() {
        mInput.clear();
    }

private:
    std::list<std::string> mInput;
    std::mutex mInputMutex;
    std::condition_variable mInputCondition;
};
