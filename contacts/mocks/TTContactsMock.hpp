#pragma once
#include <gmock/gmock.h>
#include "TTContacts.hpp"

class TTContactsMock : public TTContacts {
public:
    TTContactsMock() : TTContacts() {}
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
};
