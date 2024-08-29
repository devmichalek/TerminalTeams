#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler,
                                     TTChatHandler& chatHandler,
                                     TTNeighborsStub& neighborsStub,
                                     TTNetworkInterface interface) :
        mStopped{false},
        mContactsHandler(contactsHandler),
        mChatHandler(chatHandler),
        mNeighborsStub(neighborsStub),
        mInterface(interface) {
    LOG_INFO("Successfully constructed!");
}

TTBroadcasterChat::~TTBroadcasterChat() {
    LOG_INFO("Destructing...");
    stop();
    LOG_INFO("Successfully destructed!");
}

void TTBroadcasterChat::run() {
    LOG_INFO("Started broadcasting chat");
    while (!stopped()) {
        std::unique_lock<std::mutex> lock(mNeighborsMutex);
        const bool predicate = mNeighborsCondition.wait_for(lock, mNeighborsTimeout, [this]() {
            return !mNeighbors.empty() || stopped();
        });
        if (stopped()) {
            break;
        }
        for (auto &[id, neighbor] : mNeighbors) {
            if (!neighbor.stub) {
                const auto ipAddressAndPort = mContactsHandler.get(id).value().ipAddressAndPort;
                neighbor.stub = mNeighborsStub.createChatStub(ipAddressAndPort);
            }
            if (neighbor.stub) {
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
    }
    LOG_INFO("Stopped broadcasting chat");
}

void TTBroadcasterChat::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTBroadcasterChat::stopped() const {
    return mStopped.load();
}

bool TTBroadcasterChat::handleSend(const std::string& message) {
    const auto contactsCurrentIdOpt = mContactsHandler.current();
    if (!contactsCurrentIdOpt) {
        LOG_ERROR("Failed to handle send (cannot get current identity from contacts handler)!");
        stop();
        return false;
    }
    const auto chatCurrentIdOpt = mChatHandler.current();
    if (!chatCurrentIdOpt) {
        LOG_ERROR("Failed to handle send (cannot get current identity from chat handler)!");
        stop();
        return false;
    }
    const auto contactsCurrentId = contactsCurrentIdOpt.value();
    const auto chatCurrentId = chatCurrentIdOpt.value();
    if (chatCurrentIdOpt != contactsCurrentIdOpt) {
        LOG_ERROR("Failed to handle send (id mismatch)!");
        stop();
        return false;
    }
    if (!mContactsHandler.send(contactsCurrentId)) {
        LOG_ERROR("Failed to handle send (contacts handler send failure)!");
        stop();
        return false;
    }
    if (!mChatHandler.send(chatCurrentId, message, std::chrono::system_clock::now())) {
        LOG_ERROR("Failed to handle send (contacts handler send failure)!");
        stop();
        return false;
    }
    const auto contactsCurrentEntryOpt = mContactsHandler.get(contactsCurrentId);
    if (!contactsCurrentEntryOpt) {
        LOG_ERROR("Failed to handle send (contacts handler get failure)!");
        stop();
        return false;
    }
    const auto requestedIpAddress = contactsCurrentEntryOpt.value().ipAddressAndPort;
    if (requestedIpAddress == mInterface.getIpAddressAndPort()) {
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
        newNeighbor.second.pendingMessages.push_back(message);
        mNeighbors.insert(lb, std::move(newNeighbor));
        LOG_INFO("Success, inserted new neighbor to the map, inserted new message!");
    }
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
    if (!mContactsHandler.receive(id.value())) {
        LOG_ERROR("Failed to handle reception on contacts receive");
        stop();
        return false;
    }
    if (!mChatHandler.receive(id.value(), request.message, std::chrono::system_clock::now())) {
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
