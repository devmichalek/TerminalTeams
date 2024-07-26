#pragma once
#include <gmock/gmock.h>
#include "TTTextBox.hpp"

class TTTextBoxMock : public TTTextBox {
public:
    TTTextBoxMock() : TTTextBox() {}
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
};
