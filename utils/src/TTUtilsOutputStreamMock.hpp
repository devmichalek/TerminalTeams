#pragma once
#include <gmock/gmock.h>
#include "TTUtilsOutputStream.hpp"

class TTUtilsOutputStreamMock : public TTUtilsOutputStream {
 public:
    MOCK_METHOD(const TTUtilsOutputStream&, print, (const char*), (const, override));
    MOCK_METHOD(const TTUtilsOutputStream&, print, (std::string), (const, override));
    MOCK_METHOD(const TTUtilsOutputStream&, endl, (), (const, override));
    MOCK_METHOD(const TTUtilsOutputStream&, flush, (), (const, override));
};
