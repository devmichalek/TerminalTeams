#pragma once
#include <gmock/gmock.h>
#include "TTContactsConsumer.hpp"

class TTContactsConsumerMock : public TTContactsConsumer {
 public:
    explicit TTContactsConsumerMock(const std::string& sharedMemoryName,
        const std::string& dataConsumedSemName,
        const std::string& dataProducedSemName,
        std::shared_ptr<TTContactsSyscall> syscall) :
        TTContactsConsumer(sharedMemoryName, dataConsumedSemName, dataProducedSemName, std::move(syscall)) {}
    MOCK_METHOD(bool, init, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(std::unique_ptr<TTContactsMessage>, get, (long attempts, long timeoutMs), (override));
    MOCK_METHOD(bool, alive, (), (const, override));
};
