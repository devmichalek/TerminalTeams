#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler,
                                     TTChatHandler& chatHandler,
                                     TTNeighborsStub& neighborsStub,
                                     TTNetworkInterface networkInterface) :
        mContactsHandler(contactsHandler),
        mChatHandler(chatHandler),
        mNeighborsStub(neighborsStub),
        mNetworkInterface(networkInterface),
        mNeighborsFlag{false},
        mInactivityTimerFactory(std::chrono::milliseconds(3000), std::chrono::milliseconds(6000)){
    LOG_INFO("Successfully constructed!");
}

TTBroadcasterChat::~TTBroadcasterChat() {
    LOG_INFO("Destructing...");
    stop();
    LOG_INFO("Successfully destructed!");
}

void TTBroadcasterChat::run() {
    LOG_INFO("Started broadcasting chat");
    while (!isStopped()) {
        mNeighborsFlag.store(false);
        std::unique_lock<std::mutex> lock(mNeighborsMutex);
        const bool predicate = mNeighborsCondition.wait_for(lock, mInactivityTimerFactory.min(), [this]() {
            return mNeighborsFlag.load() || isStopped();
        });
        for (auto &[id, neighbor] : mNeighbors) {
            if (isStopped()) {
                break;
            }
            if (!neighbor.timer.expired()) {
                continue;
            }
            neighbor.timer.kick();
            const auto neighborsEntry = mContactsHandler.get(id).value();
            if (neighborsEntry.state.isInactive()) {
                continue;
            }
            if (!neighbor.stub) {
                const auto ipAddressAndPort = neighborsEntry.ipAddressAndPort;
                neighbor.stub = mNeighborsStub.createChatStub(ipAddressAndPort);
            }
            if (!neighbor.stub) [[unlikely]] {
                continue;
            }
            if (neighbor.pendingMessages.size() > 1) {
                const auto narrateRequest = TTNarrateRequest{getIdentity(), neighbor.pendingMessages};
                if (mNeighborsStub.sendNarrate(*neighbor.stub, narrateRequest).status) {
                    neighbor.pendingMessages.clear();
                }
            } else if (!neighbor.pendingMessages.empty()) {
                const auto tellRequest = TTTellRequest{getIdentity(), neighbor.pendingMessages.back()};
                if (mNeighborsStub.sendTell(*neighbor.stub, tellRequest).status) {
                    neighbor.pendingMessages.clear();
                }
            }
        }
    }
    LOG_INFO("Stopped broadcasting chat");
}

bool TTBroadcasterChat::handleSend(const std::string& message) {
    const auto contactsCurrentIdOpt = mContactsHandler.current();
    if (!contactsCurrentIdOpt) [[unlikely]] {
        LOG_ERROR("Failed to handle send (cannot get current identity from contacts handler)!");
        stop();
        return false;
    }
    const auto chatCurrentIdOpt = mChatHandler.current();
    if (!chatCurrentIdOpt) [[unlikely]] {
        LOG_ERROR("Failed to handle send (cannot get current identity from chat handler)!");
        stop();
        return false;
    }
    const auto contactsCurrentId = contactsCurrentIdOpt.value();
    const auto chatCurrentId = chatCurrentIdOpt.value();
    if (chatCurrentIdOpt != contactsCurrentIdOpt) [[unlikely]] {
        LOG_ERROR("Failed to handle send (id mismatch)!");
        stop();
        return false;
    }
    if (!mContactsHandler.send(contactsCurrentId)) [[unlikely]] {
        LOG_ERROR("Failed to handle send (contacts handler send failure)!");
        stop();
        return false;
    }
    if (!mChatHandler.send(chatCurrentId, message, std::chrono::system_clock::now())) [[unlikely]] {
        LOG_ERROR("Failed to handle send (contacts handler send failure)!");
        stop();
        return false;
    }
    const auto contactsCurrentEntryOpt = mContactsHandler.get(contactsCurrentId);
    if (!contactsCurrentEntryOpt) [[unlikely]] {
        LOG_ERROR("Failed to handle send (contacts handler get failure)!");
        stop();
        return false;
    }
    const auto requestedIpAddress = contactsCurrentEntryOpt.value().ipAddressAndPort;
    if (requestedIpAddress == mNetworkInterface.getIpAddressAndPort()) {
        LOG_INFO("Success, nothing to be send (host IP address match)!");
        return true;
    }
    std::scoped_lock lock(mNeighborsMutex);
    auto lb = mNeighbors.lower_bound(contactsCurrentId);
    if(lb != mNeighbors.end() && !(mNeighbors.key_comp()(contactsCurrentId, lb->first)))
    {
        lb->second.pendingMessages.push_back(message);
        LOG_INFO("Success, neighbor already in the map, inserted new message!");
    }
    else
    {
        auto newNeighbor = std::pair{contactsCurrentId, Neighbor{}};
        newNeighbor.second.stub = mNeighborsStub.createChatStub(requestedIpAddress);
        newNeighbor.second.timer = mInactivityTimerFactory.create();
        newNeighbor.second.timer.expire();
        newNeighbor.second.pendingMessages.push_back(message);
        mNeighbors.insert(lb, std::move(newNeighbor));
        LOG_INFO("Success, inserted new neighbor to the map, inserted new message!");
    }
    mNeighborsFlag.store(true);
    mNeighborsCondition.notify_one();
    return true;
}

bool TTBroadcasterChat::handleReceive(const TTTellRequest& request) {
    LOG_INFO("Handing reception of tell request...");
    const auto id = mContactsHandler.get(request.identity);
    if (!id) {
        LOG_WARNING("Failed to handle reception, no such identity={}", request.identity);
        return false;
    }
    if (!mContactsHandler.receive(id.value())) [[unlikely]] {
        LOG_ERROR("Failed to handle reception on contacts receive");
        stop();
        return false;
    }
    if (!mChatHandler.receive(id.value(), request.message, std::chrono::system_clock::now())) [[unlikely]] {
        LOG_ERROR("Failed to handle reception on chat receive");
        stop();
        return false;
    }
    LOG_INFO("Successfully handled reception!");
    return true;
}

bool TTBroadcasterChat::handleReceive(const TTNarrateRequest& request) {
    LOG_INFO("Handing reception of narrate request...");
    const auto id = mContactsHandler.get(request.identity);
    if (!id) {
        LOG_WARNING("Failed to handle reception, no such identity={}", request.identity);
        return false;
    }
    if (!mContactsHandler.receive(id.value())) {
        LOG_ERROR("Failed to handle reception on contacts receive");
        stop();
        return false;
    }
    for (const auto &message : request.messages) {
        if (!mChatHandler.receive(id.value(), message, std::chrono::system_clock::now())) {
            LOG_ERROR("Failed to handle reception on chat receive");
            stop();
            return false;
        }
    }
    LOG_INFO("Successfully handled reception!");
    return true;
}

std::string TTBroadcasterChat::getIdentity() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        LOG_ERROR("Failed to get identity!");
        stop();
        return {};
    }
    return opt->identity;
}
