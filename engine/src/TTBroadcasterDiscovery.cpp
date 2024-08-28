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
        mBroadcasterStub(),
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
        auto stub = mBroadcasterStub.createDiscoveryStub(neighbor);
        auto greetRequest = TTGreetRequest{getNickname(), getIdentity(), getIpAddressAndPort()};
        auto greetResponse = mBroadcasterStub.sendGreet(stub, greetRequest);
        if (greetResponse.status) {
            addNeighbor(greetResponse.nickname, greetResponse.identity, greetResponse.ipAddressAndPort);
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
                        const auto heartbeatRequest = TTHeartbeatRequest{getIdentity()};
                        const auto heartbeatResponse = mBroadcasterStub.sendHeartbeat(neighbor.stub, heartbeatRequest);
                        if (heartbeatResponse.status && !heartbeatResponse.identity.empty()) {
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

bool TTBroadcasterDiscovery::handleGreet(const TTGreetRequest& request) {
    LOG_INFO("Handling greet, new contact id={}", request.identity);
    return addNeighbor(request.nickname, request.identity, request.ipAddressAndPort);
}

bool TTBroadcasterDiscovery::handleHeartbeat(const TTHeartbeatRequest& request) {
    decltype(auto) id = mContactsHandler.get(request.identity);
    if (id == std::nullopt) {
        LOG_INFO("Handling heartbeat, ignoring not existing contact id={}", request.identity);
        return false;
    }
    LOG_INFO("Handling heartbeat, existing contact id={}", request.identity);
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

bool TTBroadcasterDiscovery::addNeighbor(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) {
    LOG_INFO("Adding neighbor...");
    if (nickname.empty() || identity.empty() || ipAddressAndPort.empty()) {
        LOG_WARNING("Rejecting neighbor (incomplete data)...");
        return false;
    }
    std::scoped_lock neighborLock(mNeighborMutex);
    decltype(auto) id = mContactsHandler.get(identity);
    if (id != std::nullopt) {
        LOG_WARNING("Rejecting neighbor (already present)...");
        return true;
    }
    if (!mContactsHandler.create(nickname, identity, ipAddressAndPort)) {
        LOG_WARNING("Rejecting neighbor (failed to create)...");
        return false;
    }
    id = mContactsHandler.get(identity);
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
    auto stub = mBroadcasterStub.createDiscoveryStub(ipAddressAndPort);
    mDynamicNeighbors.emplace(std::piecewise_construct,
        std::forward_as_tuple(id.value()),
        std::forward_as_tuple(TIMESTAMP_TIMEOUT, TIMESTAMP_TRIALS, std::move(stub)));
    LOG_INFO("Successfully added neighbor!");
    return true;
}
