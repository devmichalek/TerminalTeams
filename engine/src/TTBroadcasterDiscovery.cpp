#include "TTBroadcasterDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterDiscovery::TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                                               TTChatHandler& chatHandler,
                                               TTNetworkInterface interface,
                                               std::deque<std::string> neighbors) :
        mStopped{false},
        mContactsHandler(contactsHandler),
        mChatHandler(chatHandler),
        mInterface(interface),
        mStaticNeighbors(neighbors) {
    LOG_INFO("Successfully constructed!");
}

TTBroadcasterDiscovery::~TTBroadcasterDiscovery() {
    LOG_INFO("Destructing...");
    stop();
    LOG_INFO("Successfully destructed!");
}

void TTBroadcasterDiscovery::run() {
    LOG_INFO("Started broadcasting discovery");
    for (const auto& neighbor : mStaticNeighbors) {
        if (stopped()) {
            break;
        }
        auto greetResult = sendGreet(neighbor);
        if (greetResult) {
            addNeighbor(*greetResult);
        }
    }
    while (!stopped()) {
        std::chrono::milliseconds smallest = TIMESTAMP_TIMEOUT;
        {
            std::scoped_lock neighborLock(mNeighborMutex);
            const auto count = mDynamicNeighbors.size();
            for (auto &[id, neighbor] : mDynamicNeighbors) {
                if (neighbor.trials) {
                    if (neighbor.timestamp.expired()) {
                        const auto res = sendHeartbeat(neighbor.stub);
                        if (res && !res->identity.empty()) {
                            neighbor.trials = TIMESTAMP_TRIALS;
                            if (!mContactsHandler.activate(id)) {
                                stop();
                                break;
                            }
                        } else {
                            --neighbor.trials;
                            if (!mContactsHandler.deactivate(id)) {
                                stop();
                                break;
                            }
                        }
                        neighbor.timestamp.kick();
                    } else {
                        const auto remaining = neighbor.timestamp.remaining();
                        if (remaining < smallest) {
                            smallest = remaining;
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(smallest));
    }
    LOG_INFO("Stopped broadcasting discovery");
}

void TTBroadcasterDiscovery::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTBroadcasterDiscovery::stopped() const {
    return mStopped.load();
}

bool TTBroadcasterDiscovery::handleGreet(const TTGreetMessage& message) {
    LOG_INFO("Handling greet, new contact id={}", message.identity);
    return addNeighbor(message);
}

bool TTBroadcasterDiscovery::handleHeartbeat(const TTHeartbeatMessage& message) {
    decltype(auto) id = mContactsHandler.get(message.identity);
    if (id == std::nullopt) {
        LOG_INFO("Handling heartbeat, ignoring not existing contact id={}", message.identity);
        return false;
    }
    LOG_INFO("Handling heartbeat, existing contact id={}", message.identity);
    return true;
}

std::string TTBroadcasterDiscovery::getNickname() const {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get nickname!");
        return {};
    }
    return opt->nickname;
}

std::string TTBroadcasterDiscovery::getIdentity() const {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get identity!");
        return {};
    }
    return opt->identity;
}

std::string TTBroadcasterDiscovery::getIpAddressAndPort() const {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get IP address and port!");
        return {};
    }
    return opt->ipAddressAndPort;
}

TTBroadcasterDiscovery::UniqueStub TTBroadcasterDiscovery::createStub(const std::string& ipAddressAndPort) {
    try {
        auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
        return NeighborsDiscovery::NewStub(channel);
    } catch (...) {
        LOG_WARNING("Failed to create stub to {}!", ipAddressAndPort);
        return {};
    }
}

std::optional<TTGreetMessage> TTBroadcasterDiscovery::sendGreet(TTBroadcasterDiscovery::UniqueStub& stub) {
    try {
        GreetRequest request;
        request.set_nickname(getNickname());
        request.set_identity(getIdentity());
        request.set_ipaddressandport(getIpAddressAndPort());
        GreetReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub->Greet(&context, request, &reply);
        TTGreetMessage result;
        if (status.ok()) {
            result.nickname = reply.nickname();
            result.identity = reply.identity();
            result.ipAddressAndPort = reply.ipaddressandport();
        }
        return result;
    } catch (...) {
        LOG_ERROR("Exception occurred while sending greet!");
        return {};
    }
}

std::optional<TTGreetMessage> TTBroadcasterDiscovery::sendGreet(const std::string& ipAddressAndPort) {
    LOG_INFO("Sending greet to the {}", ipAddressAndPort);
    auto stub = createStub(ipAddressAndPort);
    if (!stub) {
        return {};
    }
    return sendGreet(stub);
}

std::optional<TTHeartbeatMessage> TTBroadcasterDiscovery::sendHeartbeat(TTBroadcasterDiscovery::UniqueStub& stub) {
    try {
        HeartbeatRequest request;
        request.set_identity(getIdentity());
        HeartbeatReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub->Heartbeat(&context, request, &reply);
        TTHeartbeatMessage result;
        if (status.ok()) {
            result.identity = reply.identity();
        }
        return result;
    } catch (...) {
        LOG_ERROR("Exception occurred while sending greet!");
        return {};
    }
}

std::optional<TTHeartbeatMessage> TTBroadcasterDiscovery::sendHeartbeat(const std::string& ipAddressAndPort) {
    LOG_INFO("Sending heartbeat to the {}", ipAddressAndPort);
    auto stub = createStub(ipAddressAndPort);
    if (!stub) {
        return {};
    }
    return sendHeartbeat(stub);
}

bool TTBroadcasterDiscovery::addNeighbor(const TTGreetMessage& message) {
    LOG_INFO("Adding neighbor...");
    if (message.nickname.empty() || message.identity.empty() || message.ipAddressAndPort.empty()) {
        LOG_WARNING("Rejecting neighbor (incomplete data)...");
        return false;
    }
    std::scoped_lock neighborLock(mNeighborMutex);
    decltype(auto) id = mContactsHandler.get(message.identity);
    if (id != std::nullopt) {
        LOG_WARNING("Rejecting neighbor (already present)...");
        return true;
    }
    if (!mContactsHandler.create(message.nickname, message.identity, message.ipAddressAndPort)) {
        LOG_WARNING("Rejecting neighbor (failed to create)...");
        return false;
    }
    id = mContactsHandler.get(message.identity);
    if (id == std::nullopt) {
        LOG_ERROR("Rejecting neighbor (cannot find new identity)...");
        stop();
        return false;
    }
    if (!mChatHandler.create(id.value())) {
        LOG_ERROR("Rejecting neighbor (cannot proceed with creation)...");
        stop();
        return false;
    }
    auto stub = createStub(message.ipAddressAndPort);
    mDynamicNeighbors.emplace(std::piecewise_construct,
        std::forward_as_tuple(id.value()),
        std::forward_as_tuple(TIMESTAMP_TIMEOUT, TIMESTAMP_TRIALS, std::move(stub)));
    LOG_INFO("Successfully added neighbor!");
    return true;
}

size_t TTBroadcasterDiscovery::getNeighborsCount() const {
    std::shared_lock neighborLock(mNeighborMutex);
    return mDynamicNeighbors.size();
}
