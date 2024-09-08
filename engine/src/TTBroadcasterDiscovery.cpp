#include "TTBroadcasterDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterDiscovery::TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                                               TTChatHandler& chatHandler,
                                               TTNeighborsStub& neighborsStub,
                                               TTNetworkInterface interface,
                                               std::deque<std::string> neighbors) :
        mStopped{false},
        mContactsHandler(contactsHandler),
        mChatHandler(chatHandler),
        mNeighborsStub(neighborsStub),
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
        auto stub = mNeighborsStub.createDiscoveryStub(neighbor);
        auto greetRequest = TTGreetRequest{getNickname(), getIdentity(), getIpAddressAndPort()};
        auto greetResponse = mNeighborsStub.sendGreet(*stub, greetRequest);
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
                if (!neighbor.timestamp.expired()) {
                    const auto remaining = neighbor.timestamp.remaining();
                    if (remaining < smallest) {
                        smallest = remaining;
                    }
                    continue;
                }
                if (neighbor.trials) {
                    if (!neighbor.stub) {
                        const auto entryOpt = mContactsHandler.get(id);
                        if (!entryOpt) {
                            LOG_ERROR("Failed to get entry using contacts handler!");
                            stop();
                            break;
                        }
                        const auto entry = entryOpt.value();
                        neighbor.stub = mNeighborsStub.createDiscoveryStub(entry.ipAddressAndPort);
                    }
                    if (neighbor.stub) {
                        const auto heartbeatRequest = TTHeartbeatRequest{getIdentity()};
                        const auto heartbeatResponse = mNeighborsStub.sendHeartbeat(*neighbor.stub, heartbeatRequest);
                        if (heartbeatResponse.status && !heartbeatResponse.identity.empty()) {
                            neighbor.trials = TIMESTAMP_TRIALS + 1;
                            if (!mContactsHandler.activate(id)) {
                                LOG_ERROR("Failed to activate using contacts handler!");
                                stop();
                                break;
                            }
                        } else {
                            if (!mContactsHandler.deactivate(id)) {
                                LOG_ERROR("Failed to deactivate using contacts handler!");
                                stop();
                                break;
                            }
                        }
                    }
                    --neighbor.trials;
                    neighbor.timestamp.kick();
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

std::string TTBroadcasterDiscovery::getNickname() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        LOG_ERROR("Failed to get nickname!");
        stop();
        return {};
    }
    return opt->nickname;
}

std::string TTBroadcasterDiscovery::getIdentity() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        LOG_ERROR("Failed to get identity!");
        stop();
        return {};
    }
    return opt->identity;
}

std::string TTBroadcasterDiscovery::getIpAddressAndPort() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        LOG_ERROR("Failed to get IP address and port!");
        stop();
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
        if (!mContactsHandler.activate(id.value())) {
            LOG_ERROR("Rejecting neighbor (failed to activate)...");
            stop();
            return false;
        }
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
    auto stub = mNeighborsStub.createDiscoveryStub(ipAddressAndPort);
    mDynamicNeighbors.emplace(std::piecewise_construct,
        std::forward_as_tuple(id.value()),
        std::forward_as_tuple(TIMESTAMP_TIMEOUT, TIMESTAMP_TRIALS, std::move(stub)));
    LOG_INFO("Successfully added new neighbor!");
    return true;
}
