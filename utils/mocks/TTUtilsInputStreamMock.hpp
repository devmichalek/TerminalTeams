#pragma once
#include <gmock/gmock.h>
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
        output = mInput.back();
        mInput.pop_back();
        return *this;
    }

    std::list<std::string> mInput;
};
