#pragma once
#include <gmock/gmock.h>
#include "TTUtilsOutputStream.hpp"

class TTUtilsOutputStreamMock : public TTUtilsOutputStream {
public:
    MOCK_METHOD(TTUtilsOutputStream&, print, (const char*), (override));
    MOCK_METHOD(TTUtilsOutputStream&, print, (std::string), (override));
    MOCK_METHOD(TTUtilsOutputStream&, endl, (), (override));
    MOCK_METHOD(TTUtilsOutputStream&, flush, (), (override));
    MOCK_METHOD(TTUtilsOutputStream&, clear, (), (override));
};
