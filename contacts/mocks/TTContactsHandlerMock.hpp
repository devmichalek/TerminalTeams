#pragma once
#include <gmock/gmock.h>
#include "TTContactsHandler.hpp"

class TTContactsHandlerMock : public TTContactsHandler {
public:
    TTContactsHandlerMock() : TTContactsHandler() {}
    MOCK_METHOD(bool, create, (const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, send, (size_t), (override));
    MOCK_METHOD(bool, receive, (size_t), (override));
    MOCK_METHOD(bool, activate, (size_t), (override));
    MOCK_METHOD(bool, deactivate, (size_t), (override));
    MOCK_METHOD(bool, select, (size_t), (override));
    MOCK_METHOD(std::optional<TTContactsHandlerEntry>, get, (size_t), (const, override));
    MOCK_METHOD(std::optional<size_t>, get, (std::string), (const, override));
    MOCK_METHOD(size_t, current, (), (const, override));
    MOCK_METHOD(size_t, size, (), (const, override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, stopped, (), (const, override));
};
