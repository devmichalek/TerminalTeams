#include "TTBroadcasterDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterDiscovery::TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                                               TTChatHandler& chatHandler,
                                               TTNeighborsStub& neighborsStub,
                                               TTNetworkInterface networkInterface,
                                               const std::deque<std::string>& neighbors) :
        mContactsHandler(contactsHandler),
        mChatHandler(chatHandler),
        mNeighborsStub(neighborsStub),
        mNetworkInterface(networkInterface),
        mInactivityTimerFactory(std::chrono::milliseconds(5000), std::chrono::milliseconds(6000)),
        mDiscoveryTimerFactory(std::chrono::milliseconds(100), std::chrono::milliseconds(1000)) {
    for (const auto &neighbor : neighbors) {
        LOG_INFO("Creating static neighbor from IP address={}", neighbor);
        mStaticNeighbors.emplace_back(mDiscoveryTimerFactory.create(), neighbor + ":" + networkInterface.getPort());
        mStaticNeighbors.back().timer.expire();
    }
    LOG_INFO("Successfully constructed!");
}

TTBroadcasterDiscovery::~TTBroadcasterDiscovery() {
    LOG_INFO("Destructing...");
    stop();
    LOG_INFO("Successfully destructed!");
}

void TTBroadcasterDiscovery::run() {
    LOG_INFO("Started broadcasting discovery");
    auto staticNeighborsResult = std::async(std::launch::async, std::bind(&TTBroadcasterDiscovery::resolveStaticNeighbors, this));
    auto dynamicNeighborsResult = std::async(std::launch::async, std::bind(&TTBroadcasterDiscovery::resolveDynamicNeighbors, this));
    if (staticNeighborsResult.valid()) {
        staticNeighborsResult.wait();
    }
    if (dynamicNeighborsResult.valid()) {
        dynamicNeighborsResult.wait();
    }
    LOG_INFO("Stopped broadcasting discovery");
}

bool TTBroadcasterDiscovery::handleGreet(const TTGreetRequest& request) {
    LOG_INFO("Handling greet, new contact id={}", request.identity);
    return addNeighbor(request.nickname, request.identity, request.ipAddressAndPort, nullptr);
}

bool TTBroadcasterDiscovery::handleHeartbeat(const TTHeartbeatRequest& request) {
    decltype(auto) id = mContactsHandler.get(request.identity);
    if (id == std::nullopt) {
        LOG_WARNING("Handling heartbeat, ignoring not existing contact id={}", request.identity);
        return false;
    }
    LOG_INFO("Handling heartbeat, existing contact id={}", request.identity);
    if (!mContactsHandler.activate(id.value())) [[unlikely]] {
        LOG_ERROR("Handle heartbeat error (failed to activate)...");
        stop();
        return false;
    }
    return true;
}

std::string TTBroadcasterDiscovery::getNickname() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) [[unlikely]] {
        LOG_ERROR("Failed to get nickname!");
        stop();
        return {};
    }
    return opt->nickname;
}

std::string TTBroadcasterDiscovery::getIdentity() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) [[unlikely]] {
        LOG_ERROR("Failed to get identity!");
        stop();
        return {};
    }
    return opt->identity;
}

std::string TTBroadcasterDiscovery::getIpAddressAndPort() {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) [[unlikely]] {
        LOG_ERROR("Failed to get IP address and port!");
        stop();
        return {};
    }
    return opt->ipAddressAndPort;
}

void TTBroadcasterDiscovery::resolveStaticNeighbors() {
    LOG_INFO("Started resolving static neighbors");
    do {
        auto smallest = mDiscoveryTimerFactory.max();
        bool resolved = true;
        for (auto& neighbor : mStaticNeighbors) {
            if (isStopped()) {
                break;
            }
            if (neighbor.trials > 0) {
                resolved = false;
                if (neighbor.timer.expired()) {
                    --neighbor.trials;
                    auto stub = mNeighborsStub.createDiscoveryStub(neighbor.ipAddressAndPort);
                    if (stub) {
                        auto greetRequest = TTGreetRequest{getNickname(), getIdentity(), getIpAddressAndPort()};
                        auto greetResponse = mNeighborsStub.sendGreet(*stub, greetRequest);
                        if (greetResponse.status) {
                            if (addNeighbor(greetResponse.nickname, greetResponse.identity, greetResponse.ipAddressAndPort, std::move(stub))) {
                                neighbor.trials = 0;
                            }
                        }
                    }
                    neighbor.timer.kick();
                } else {
                    const auto remaining = neighbor.timer.remaining();
                    if (remaining < smallest) {
                        smallest = remaining;
                    }
                }
            }
        }
        if (resolved) {
            break;
        }
    } while (!isStopped());
    LOG_INFO("Stopped resolving static neighbors");
}

void TTBroadcasterDiscovery::resolveDynamicNeighbors() {
    LOG_INFO("Started resolving dynamic neighbors");
    while (!isStopped()) {
        auto smallest = mInactivityTimerFactory.max();
        {
            std::scoped_lock neighborLock(mNeighborMutex);
            for (auto &[id, neighbor] : mDynamicNeighbors) {
                if (isStopped()) {
                    break;
                }
                if (!neighbor.timer.expired()) {
                    const auto remaining = neighbor.timer.remaining();
                    if (remaining < smallest) {
                        smallest = remaining;
                    }
                    continue;
                }
                if (neighbor.trials) {
                    if (!neighbor.stub) {
                        const auto entryOpt = mContactsHandler.get(id);
                        if (!entryOpt) [[unlikely]] {
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
                            neighbor.trials = DynamicNeighbor::inactivityTrials + 1;
                            if (!mContactsHandler.activate(id)) [[unlikely]] {
                                LOG_ERROR("Failed to activate using contacts handler!");
                                stop();
                                break;
                            }
                        } else {
                            if (!mContactsHandler.deactivate(id)) [[unlikely]] {
                                LOG_ERROR("Failed to deactivate using contacts handler!");
                                stop();
                                break;
                            }
                        }
                    }
                    --neighbor.trials;
                    neighbor.timer.kick();
                }
            }
        }
        LOG_INFO("Inactivity count={}ms", smallest.count());
        std::this_thread::sleep_for(smallest);
    }
    LOG_INFO("Stopped resolving dynamic neighbors");
}

bool TTBroadcasterDiscovery::addNeighbor(const std::string& nickname,
    const std::string& identity,
    const std::string& ipAddressAndPort,
    TTUniqueDiscoveryStub stub) {
    LOG_INFO("Adding neighbor...");
    if (nickname.empty() || identity.empty() || ipAddressAndPort.empty()) {
        LOG_WARNING("Rejecting neighbor (incomplete data)...");
        return false;
    }
    std::scoped_lock neighborLock(mNeighborMutex);
    decltype(auto) id = mContactsHandler.get(identity);
    if (id != std::nullopt) {
        if (!mContactsHandler.activate(id.value())) [[unlikely]] {
            LOG_ERROR("Rejecting neighbor (failed to activate)...");
            stop();
            return false;
        }
        LOG_WARNING("Rejecting neighbor (already present)...");
        return true;
    }
    if (!mContactsHandler.create(nickname, identity, ipAddressAndPort)) [[unlikely]] {
        LOG_WARNING("Rejecting neighbor (failed to create)...");
        return false;
    }
    id = mContactsHandler.get(identity);
    if (id == std::nullopt) [[unlikely]] {
        LOG_ERROR("Rejecting neighbor (cannot find new identity)...");
        stop();
        return false;
    }
    if (!mChatHandler.create(id.value())) [[unlikely]] {
        LOG_ERROR("Rejecting neighbor (cannot proceed with creation)...");
        stop();
        return false;
    }
    auto timer = mInactivityTimerFactory.create();
    timer.expire();
    mDynamicNeighbors.emplace(std::piecewise_construct,
        std::forward_as_tuple(id.value()),
        std::forward_as_tuple(timer, std::move(stub)));
    LOG_INFO("Successfully added new neighbor!");
    return true;
}
