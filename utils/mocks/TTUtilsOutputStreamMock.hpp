#pragma once
#include <gmock/gmock.h>
#include "TTUtilsOutputStream.hpp"

class TTUtilsOutputStreamMock : public TTUtilsOutputStream {
public:
    TTUtilsOutputStreamMock() = default;
    virtual ~TTUtilsOutputStreamMock() {}
    TTUtilsOutputStreamMock(const TTUtilsOutputStreamMock&) = default;
    TTUtilsOutputStreamMock(TTUtilsOutputStreamMock&&) = default;
    TTUtilsOutputStreamMock& operator=(const TTUtilsOutputStreamMock&) = default;
    TTUtilsOutputStreamMock& operator=(TTUtilsOutputStreamMock&&) = default;

    virtual TTUtilsOutputStream& print(const char* cmessage) {
        mOutput.back().append(cmessage);
        return *this;
    }

    virtual TTUtilsOutputStream& print(const std::string& message) {
        mOutput.back().append(message);
        return *this;
    }

    virtual TTUtilsOutputStream& endl() {
        mOutput.back().append("\n");
        return *this;
    }

    virtual TTUtilsOutputStream& flush() {
        mOutput.back().append("\r");
        return *this;
    }

    virtual TTUtilsOutputStream& clear() {
        mOutput.push_back("");
        return *this;
    }

    std::vector<std::string> mOutput;
};
