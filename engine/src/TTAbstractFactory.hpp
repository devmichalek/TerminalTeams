#pragma once
#include "TTNeighborsServiceChat.hpp"
#include "TTNeighborsServiceDiscovery.hpp"
#include "TTBroadcasterChat.hpp"
#include "TTBroadcasterDiscovery.hpp"
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTTextBoxHandler.hpp"
#include "TTServer.hpp"

class TTAbstractFactory
{
public:
    TTAbstractFactory(
        TTContactsSettings& contactsSettings,
        TTChatSettings& chatSettings,
        TTTextBoxSettings& textBoxSettings) :
            mContactsSettings(contactsSettings),
            mChatSettings(chatSettings),
            mTextBoxSettings(textBoxSettings) {}
    virtual ~TTAbstractFactory() {}
    TTAbstractFactory(const TTAbstractFactory&) = default;
    TTAbstractFactory(TTAbstractFactory&&) = default;
    TTAbstractFactory& operator=(const TTAbstractFactory&) = default;
    TTAbstractFactory& operator=(TTAbstractFactory&&) = default;

    [[nodiscard]] virtual std::unique_ptr<TTContactsHandler> createContactsHandler() const {
        return std::make_unique<TTContactsHandler>(mContactsSettings);
    }

    [[nodiscard]] virtual std::unique_ptr<TTChatHandler> createChatHandler() const {
        return std::make_unique<TTChatHandler>(mChatSettings);
    }

    [[nodiscard]] virtual std::unique_ptr<TTTextBoxHandler> createTextBoxHandler(
            TTTextBoxCallbackMessageSent callbackMessageSent,
            TTTextBoxCallbackContactSwitch callbackContactsSwitch) const {
        return std::make_unique<TTTextBoxHandler>(mTextBoxSettings, callbackMessageSent, callbackContactsSwitch);
    }

    [[nodiscard]] virtual std::unique_ptr<TTNeighborsStub> createNeighborsStub() const {
        return std::make_unique<TTNeighborsStub>();
    }

    [[nodiscard]] virtual std::unique_ptr<TTBroadcasterChat> createBroadcasterChat(
            TTContactsHandler& contactsHandler,
            TTChatHandler& chatHandler,
            TTNeighborsStub& neighborsStub,
            TTNetworkInterface networkInterface) const {
        return std::make_unique<TTBroadcasterChat>(contactsHandler, chatHandler, neighborsStub, networkInterface);
    }

    [[nodiscard]] virtual std::unique_ptr<TTBroadcasterDiscovery> createBroadcasterDiscovery(
            TTContactsHandler& contactsHandler,
            TTChatHandler& chatHandler,
            TTNeighborsStub& neighborsStub,
            TTNetworkInterface networkInterface,
            const std::deque<std::string>& neighbors) const {
        return std::make_unique<TTBroadcasterDiscovery>(contactsHandler, chatHandler, neighborsStub, networkInterface, neighbors);
    }

    [[nodiscard]] virtual std::unique_ptr<TTNeighborsServiceChat> createNeighborsServiceChat(
            TTBroadcasterChat& chat) const {
        return std::make_unique<TTNeighborsServiceChat>(chat);
    }

    [[nodiscard]] virtual std::unique_ptr<TTNeighborsServiceDiscovery> createNeighborsServiceDiscovery(
            TTBroadcasterDiscovery& discovery) const {
        return std::make_unique<TTNeighborsServiceDiscovery>(discovery);
    }

    [[nodiscard]] virtual std::unique_ptr<TTServer> createServer(
            const std::string& ipAddressAndPort,
            TTNeighborsServiceChat& chat,
            TTNeighborsServiceDiscovery& discovery) const {
        return std::make_unique<TTServer>(ipAddressAndPort, chat, discovery);
    }

private:
    TTContactsSettings& mContactsSettings;
    TTChatSettings& mChatSettings;
    TTTextBoxSettings& mTextBoxSettings;
};
