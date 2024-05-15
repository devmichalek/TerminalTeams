#pragma once
#include <gmock/gmock.h>
#include "TTUtilsSharedMem.hpp"

class TTUtilsSharedMemMock : public TTUtilsSharedMem {
 public:
    MOCK_METHOD(bool, open, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, receive, (void* memory, long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, alive, (), (const, override));
};
