#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler) :
        mStopped{false}, mContactsHandler(contactsHandler), mChatHandler(chatHandler) {
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
        for (size_t j = 0; j < mNeighbors.size(); ++j) {
            if (!mNeighbors[j].stub) {
                const auto ipAddressAndPort = mContactsHandler.get(j).value().ipAddressAndPort;
                mNeighbors[j].stub = createStub(ipAddressAndPort);
            }
            if (mNeighbors[j].stub) {
                if (mNeighbors[j].pendingMessages.size() > 1) {
                    if (sendNarrate(mNeighbors[j])) {
                        mNeighbors[j].pendingMessages.clear();
                    }
                } else if (!mNeighbors[j].pendingMessages.empty()) {
                    if (sendTell(mNeighbors[j])) {
                        mNeighbors[j].pendingMessages.clear();
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
    bool result = mChatHandler.send(chatCurrent, message, now);
    result &= mContactsHandler.send(contactsCurrent);
    result &= (chatCurrent == contactsCurrent);
    if (!result) {
        LOG_ERROR("Failed to handle send (id mismatch)!");
        stop();
        return false;
    }
    std::scoped_lock lock(mNeighborsMutex);
    if (contactsCurrent == mNeighbors.size()) {
        Neighbor neighbor;
        const auto ipAddressAndPort = mContactsHandler.get(contactsCurrent).value().ipAddressAndPort;
        mNeighbors.push_back({});
        mNeighbors.back().stub = createStub(ipAddressAndPort);
    } else if (contactsCurrent > mNeighbors.size()) {
        LOG_ERROR("Failed to handle send (id is exceeded)!");
        stop();
        return false;
    }
    mNeighbors[contactsCurrent].pendingMessages.push_back(message);
    mNeighborsCondition.notify_one();
    return true;
}

bool TTBroadcasterChat::handleTell(const TTNarrateMessage& message) {
    LOG_INFO("Handling tell...");
    const auto id = mContactsHandler.get(message.identity);
    if (!id) {
        LOG_WARNING("Failed to handle tell, no such identity={}", message.identity);
        return false;
    }
    if (!mContactsHandler.receive(id.value())) {
        LOG_ERROR("Failed to handle tell on contacts receive");
        stop();
    }
    if (!mChatHandler.receive(id.value(), message.message, std::chrono::system_clock::now())) {
        LOG_ERROR("Failed to handle tell on chat receive");
        stop();
    }
    return true;
}

bool TTBroadcasterChat::handleNarrate(const TTNarrateMessages& messages) {
    LOG_INFO("Handling narrate...");
    for (const auto &message : messages) {
        const auto id = mContactsHandler.get(message.identity);
        if (!id) {
            LOG_WARNING("Failed to handle narrate, no such identity={}", message.identity);
            return false;
        }
        if (!mContactsHandler.receive(id.value())) {
            LOG_ERROR("Failed to handle narrate on contacts receive");
            stop();
        }
        if (!mChatHandler.receive(id.value(), message.message, std::chrono::system_clock::now())) {
            LOG_ERROR("Failed to handle narrate on chat receive");
            stop();
        }
    }
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
                LOG_ERROR("Exception occurred while sending narrate (broken stream)!");
                return false;
            }
        }
        grpc::Status status = writer->Finish();
        if (status.ok()) {
            return true;
        }
        return false;
    } catch (...) {
        LOG_ERROR("Exception occurred while sending narrate!");
    }
    return false;
}
