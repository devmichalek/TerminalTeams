#pragma once
#include <gmock/gmock.h>

class TTServerMock : public TTServer {
public:
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, isStopped, (), (const, override));
};
