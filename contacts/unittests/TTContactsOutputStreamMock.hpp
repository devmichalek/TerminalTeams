#pragma once
#include <gmock/gmock.h>
#include "TTContactsOutputStream.hpp"

class TTContactsOutputStreamMock : public TTContactsOutputStream {
 public:
    MOCK_METHOD(const TTContactsOutputStream&, print, (const char*), (const, override));
    MOCK_METHOD(const TTContactsOutputStream&, print, (std::string), (const, override));
    MOCK_METHOD(const TTContactsOutputStream&, endl, (), (const, override));
    MOCK_METHOD(const TTContactsOutputStream&, flush, (), (const, override));
};
