#pragma once
#include <gmock/gmock.h>
#include "TTAbstractFactory.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTChatSettingsMock.hpp"
#include "TTTextBoxSettingsMock.hpp"

class TTAbstractFactoryMock : public TTAbstractFactory {
public:
    TTAbstractFactoryMock() : TTAbstractFactory(mContactsSettings, mChatSettings, mTextBoxSettings) {}
    MOCK_METHOD(std::unique_ptr<TTContactsHandler>, createContactsHandler, (), (const, override));
    MOCK_METHOD(std::unique_ptr<TTChatHandler>, createChatHandler, (), (const, override));
    MOCK_METHOD(std::unique_ptr<TTTextBoxHandler>, createTextBoxHandler, (
        TTTextBoxCallbackMessageSent callbackMessageSent,
        TTTextBoxCallbackContactSelect callbackContactsSelect), (const, override));
    MOCK_METHOD(std::unique_ptr<TTNeighborsStub>, createNeighborsStub, (), (const, override));
    MOCK_METHOD(std::unique_ptr<TTBroadcasterChat>, createBroadcasterChat, (
        TTContactsHandler& contactsHandler,
        TTChatHandler& chatHandler,
        TTNeighborsStub& neighborsStub,
        TTNetworkInterface networkInterface), (const, override));
    MOCK_METHOD(std::unique_ptr<TTBroadcasterDiscovery>, createBroadcasterDiscovery, (
        TTContactsHandler& contactsHandler,
        TTChatHandler& chatHandler,
        TTNeighborsStub& neighborsStub,
        TTNetworkInterface networkInterface,
        const std::deque<std::string>& neighbors), (const, override));
    MOCK_METHOD(std::unique_ptr<TTNeighborsServiceChat>, createNeighborsServiceChat, (TTBroadcasterChat& chat), (const, override));
    MOCK_METHOD(std::unique_ptr<TTNeighborsServiceDiscovery>, createNeighborsServiceDiscovery, (TTBroadcasterDiscovery& discovery), (const, override));
    MOCK_METHOD(std::unique_ptr<TTServer>, createServer, (
        const std::string& ipAddressAndPort,
        TTNeighborsServiceChat& chat,
        TTNeighborsServiceDiscovery& discovery), (const, override));
private:
    TTContactsSettingsMock mContactsSettings;
    TTChatSettingsMock mChatSettings;
    TTTextBoxSettingsMock mTextBoxSettings;
};
