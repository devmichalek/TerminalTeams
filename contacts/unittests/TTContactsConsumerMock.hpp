#pragma once
#include <gmock/gmock.h>
#include "TTContactsConsumer.hpp"

class TTContactsConsumerMock : public TTContactsConsumer {
 public:
    MOCK_METHOD(bool, init, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(std::unique_ptr<TTContactsMessage>, get, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, alive, (), (const, override));
};
