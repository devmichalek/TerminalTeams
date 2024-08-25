#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler, TTNetworkInterface interface) :
        mStopped{false}, mContactsHandler(contactsHandler), mChatHandler(chatHandler), mInterface(interface) {
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
                neighbor.stub = createStub(ipAddressAndPort);
            }
            if (neighbor.stub) {
                if (neighbor.pendingMessages.size() > 1) {
                    if (sendNarrate(neighbor)) {
                        neighbor.pendingMessages.clear();
                    }
                } else if (!neighbor.pendingMessages.empty()) {
                    if (sendTell(neighbor)) {
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
    const auto now = std::chrono::system_clock::now();
    const auto chatCurrent = mChatHandler.current().value();
    const auto contactsCurrent = mContactsHandler.current().value();
    const auto contactsCurrentEntry = mContactsHandler.get(contactsCurrent).value();
    bool result = mChatHandler.send(chatCurrent, message, now);
    result &= mContactsHandler.send(contactsCurrent);
    result &= (chatCurrent == contactsCurrent);
    if (!result) {
        LOG_ERROR("Failed to handle send (id mismatch)!");
        stop();
        return false;
    }
    if (contactsCurrentEntry.ipAddressAndPort == mInterface.getIpAddressAndPort()) {
        LOG_INFO("Success, nothing to be send!");
        return true;
    }
    std::scoped_lock lock(mNeighborsMutex);
    auto lb = mNeighbors.lower_bound(contactsCurrent);
    if(lb != mNeighbors.end() && !(mNeighbors.key_comp()(contactsCurrent, lb->first)))
    {
        lb->second.pendingMessages.push_back(message);
        LOG_INFO("Success, neighbor already in the map, inserted new message!");
    }
    else
    {
        mNeighbors.insert(lb, decltype(mNeighbors)::value_type(contactsCurrent, Neighbor{}));
        lb->second.stub = createStub(contactsCurrentEntry.ipAddressAndPort);
        lb->second.pendingMessages.push_back(message);
        LOG_INFO("Success, inserted new neighbor to the map, inserted new message!");
    }
    mNeighborsCondition.notify_one();
    return true;
}

bool TTBroadcasterChat::handleReceive(const TTNarrateMessage& message) {
    LOG_INFO("Handing reception of one message...");
    const auto id = mContactsHandler.get(message.identity);
    if (!id) {
        LOG_WARNING("Failed to handle reception, no such identity={}", message.identity);
        return false;
    }
    if (!mContactsHandler.receive(id.value())) {
        LOG_ERROR("Failed to handle reception on contacts receive");
        stop();
    }
    if (!mChatHandler.receive(id.value(), message.message, std::chrono::system_clock::now())) {
        LOG_ERROR("Failed to handle reception on chat receive");
        stop();
    }
    LOG_INFO("Successfully handled reception!");
    return true;
}

bool TTBroadcasterChat::handleReceive(const TTNarrateMessages& messages) {
    LOG_INFO("Handing reception of many messages...");
    for (const auto &message : messages) {
        const auto id = mContactsHandler.get(message.identity);
        if (!id) {
            LOG_WARNING("Failed to handle reception, no such identity={}", message.identity);
            return false;
        }
        if (!mContactsHandler.receive(id.value())) {
            LOG_ERROR("Failed to handle reception on contacts receive");
            stop();
        }
        if (!mChatHandler.receive(id.value(), message.message, std::chrono::system_clock::now())) {
            LOG_ERROR("Failed to handle reception on chat receive");
            stop();
        }
    }
    LOG_INFO("Successfully handled reception!");
    return true;
}

std::string TTBroadcasterChat::getIdentity() const {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get identity!");
    }
    return opt->identity;
}

TTBroadcasterChat::UniqueStub TTBroadcasterChat::createStub(const std::string& ipAddressAndPort) {
    try {
        auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
        return NeighborsChat::NewStub(channel);
    } catch (...) {
        LOG_WARNING("Failed to create stub to {}!", ipAddressAndPort);
        return {};
    }
}

bool TTBroadcasterChat::sendTell(const TTBroadcasterChat::Neighbor& neighbor) {
    try {
        TellRequest request;
        request.set_identity(getIdentity());
        request.set_message(neighbor.pendingMessages.back());
        TellReply reply;
        grpc::ClientContext context;
        grpc::Status status = neighbor.stub->Tell(&context, request, &reply);
        if (status.ok()) {
            return true;
        }
        LOG_ERROR("Error status received on send tell!");
        return false;
    } catch (...) {
        LOG_ERROR("Exception occurred while sending tell!");
    }
    return false;
}

bool TTBroadcasterChat::sendNarrate(const TTBroadcasterChat::Neighbor& neighbor) {
    try {
        grpc::ClientContext context;
        NarrateReply reply;
        std::unique_ptr<grpc::ClientWriter<NarrateRequest>> writer(
            neighbor.stub->Narrate(&context, &reply));
        const auto identity = getIdentity();
        for (const auto &message : neighbor.pendingMessages) {
            NarrateRequest request;
            request.set_identity(identity);
            request.set_message(message);
            if (!writer->Write(request)) {
                LOG_ERROR("Error occurred while sending narrate (broken stream)!");
                return false;
            }
        }
        grpc::Status status = writer->Finish();
        if (status.ok()) {
            return true;
        }
        LOG_ERROR("Error status received on send narrate!");
        return false;
    } catch (...) {
        LOG_ERROR("Exception occurred while sending narrate!");
    }
    return false;
}
