#pragma once
#include <gmock/gmock.h>
#include "TTTextBoxHandler.hpp"

class TTTextBoxHandlerMock : public TTTextBoxHandler {
public:
    TTTextBoxHandlerMock() : TTTextBoxHandler() {}
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, isStopped, (), (const, override));
};
