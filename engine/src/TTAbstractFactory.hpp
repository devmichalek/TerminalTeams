#pragma once
#include "TTNeighborsServiceChat.hpp"
#include "TTNeighborsServiceDiscovery.hpp"
#include "TTBroadcasterChat.hpp"
#include "TTBroadcasterDiscovery.hpp"
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTTextBoxHandler.hpp"
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

class TTAbstractFactory
{
public:
    TTAbstractFactory(const TTContactsSettings& contactsSettings,
        const TTChatSettings& chatSettings,
        const TTTextBoxSettings& textBoxSettings) :
            mContactsSettings(contactsSettings),
            mChatSettings(chatSettings),
            mTextBoxSettings(textBoxSettings) {}
    virtual ~TTAbstractFactory() {}
    TTAbstractFactory(const TTAbstractFactory&) = default;
    TTAbstractFactory(TTAbstractFactory&&) = default;
    TTAbstractFactory& operator=(const TTAbstractFactory&) = default;
    TTAbstractFactory& operator=(TTAbstractFactory&&) = default;

    virtual std::unique_ptr<TTContactsHandler> createContactsHandler() const {
        return std::make_unique<TTContactsHandler>(mContactsSettings);
    }

    virtual std::unique_ptr<TTChatHandler> createChatHandler() const {
        return std::make_unique<TTChatHandler>(mChatSettings);
    }

    virtual std::unique_ptr<TTTextBoxHandler> createTextBoxHandler(
            TTTextBoxCallbackMessageSent callbackMessageSent,
            TTTextBoxCallbackContactSwitch callbackContactsSwitch) const {
        return std::make_unique<TTTextBoxHandler>(mTextBoxSettings, callbackMessageSent, callbackContactsSwitch);
    }

    virtual std::unique_ptr<TTNeighborsStub> createNeighborsStub() const {
        return std::make_unique<TTNeighborsStub>();
    }

    virtual std::unique_ptr<TTBroadcasterChat> createBroadcasterChat(TTContactsHandler& contactsHandler,
            TTChatHandler& chatHandler,
            TTNeighborsStub& neighborsStub,
            TTNetworkInterface networkInterface) const {
        return std::make_unique<TTBroadcasterChat>(contactsHandler, chatHandler, neighborsStub, networkInterface);
    }

    virtual std::unique_ptr<TTBroadcasterDiscovery> createBroadcasterDiscovery(TTContactsHandler& contactsHandler,
            TTChatHandler& chatHandler,
            TTNeighborsStub& neighborsStub,
            TTNetworkInterface networkInterface,
            const std::deque<std::string>& neighbors) const {
        return std::make_unique<TTBroadcasterDiscovery>(contactsHandler, chatHandler, neighborsStub, networkInterface, neighbors);
    }

    virtual std::unique_ptr<TTNeighborsServiceChat> createNeighborsServiceChat(TTBroadcasterChat& chat) const {
        return std::make_unique<TTNeighborsServiceChat>(chat);
    }

    virtual std::unique_ptr<TTNeighborsServiceDiscovery> createNeighborsServiceDiscovery(TTBroadcasterDiscovery& discovery) const {
        return std::make_unique<TTNeighborsServiceDiscovery>(discovery);
    }

    virtual std::unique_ptr<grpc::Server> createServer(const std::string& ipAddressAndPort,
            TTNeighborsServiceChat& chat, TTNeighborsServiceDiscovery& discovery) const {
        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        grpc::ServerBuilder builder;
        builder.AddListeningPort(ipAddressAndPort, grpc::InsecureServerCredentials());
        builder.RegisterService(&chat);
        builder.RegisterService(&discovery);
        return std::unique_ptr<grpc::Server>(builder.BuildAndStart());
    }
private:
    TTContactsSettings mContactsSettings;
    TTChatSettings mChatSettings;
    TTTextBoxSettings mTextBoxSettings;
};
