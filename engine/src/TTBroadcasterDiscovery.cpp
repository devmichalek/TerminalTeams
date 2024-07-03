#include "TTBroadcasterDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterDiscovery::TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                                               TTNetworkInterface interface,
                                               std::deque<std::string> neighbors) :
        mStopped{false}, mInterface(interface), mContactsHandler(contactsHandler) {
    LOG_INFO("Constructing...");
    for (const auto& neighbor : neighbors) {
        auto greetResult = sendGreet(neighbor);
        addNeighbor(greetResult);
    }
    LOG_INFO("Successfully constructed!");
}

TTBroadcasterDiscovery::~TTBroadcasterDiscovery() {
    LOG_INFO("Destructing...");
    LOG_INFO("Successfully destructed!");
}

void TTBroadcasterDiscovery::run(const size_t neighborOffset) {
    LOG_INFO("Started broadcasting discovery");
    while (!stopped()) {
        size_t count = getNeighborsCount();
        std::chrono::milliseconds smallest = TIMESTAMP_TIMEOUT;
        for (size_t i = 0; i < count; ++i) {
            if (mTimestampTrials[i]) {
                if (mTimestamps[i].expired()) {
                    if (!sendHeartbeat(mStubs[i].get()).identity.empty()) {
                        mTimestampTrials[i] = TIMESTAMP_TRIALS;
                        if (!mContactsHandler.activate(i + neighborOffset)) {
                            stop();
                            break;
                        }
                    } else {
                        --mTimestampTrials[i];
                        if (!mContactsHandler.deactivate(i + neighborOffset)) {
                            stop();
                            break;
                        }
                    }
                    mTimestamps[i].kick();
                } else {
                    smallest = mTimestamps[i].remaining();
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
    decltype(auto) id = mContactsHandler.get(message.identity);
    if (id == std::nullopt) {
        LOG_INFO("Handling greet, new contact id={}", message.identity);
        if (!addNeighbor(message)) {
            LOG_ERROR("Handling greet, failed to create contact id={}", message.identity);
            stop();
            return false;
        }
    } else {
        LOG_INFO("Handling greet, existing contact id={}", message.identity);
        // to do: inform end user that we will narrate soon!
    }

    return true;
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

const std::string& TTBroadcasterDiscovery::getNickname() const {
    decltype(auto) opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get nickname!");
    }
    return opt->nickname;
}

const std::string& TTBroadcasterDiscovery::getIdentity() const {
    decltype(auto) opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get identity!");
    }
    return opt->identity;
}

const std::string& TTBroadcasterDiscovery::getIpAddressAndPort() const {
    decltype(auto) opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get IP address and port!");
    }
    return opt->ipAddressAndPort;
}

std::unique_ptr<NeighborsDiscovery::Stub> TTBroadcasterDiscovery::createStub(const std::string& ipAddressAndPort) {
    auto channel = grpc::CreateChannel(ipAddressAndPort, grpc::InsecureChannelCredentials());
    return NeighborsDiscovery::NewStub(channel);
}

TTGreetMessage TTBroadcasterDiscovery::sendGreet(NeighborsDiscovery::Stub* stub) {
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
}

TTGreetMessage TTBroadcasterDiscovery::sendGreet(const std::string& ipAddressAndPort) {
    auto stub = createStub(ipAddressAndPort);
    return sendGreet(stub.get());
}

TTHeartbeatMessage TTBroadcasterDiscovery::sendHeartbeat(NeighborsDiscovery::Stub* stub) {
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
}

TTHeartbeatMessage TTBroadcasterDiscovery::sendHeartbeat(const std::string& ipAddressAndPort) {
    auto stub = createStub(ipAddressAndPort);
    return sendHeartbeat(stub.get());
}

bool TTBroadcasterDiscovery::addNeighbor(const TTGreetMessage& message) {
    if (message.nickname.empty() || message.identity.empty() || message.ipAddressAndPort.empty()) {
        return false;
    }

    {
        std::scoped_lock neighborLock(mNeighborMutex);
        if (mContactsHandler.create(message.nickname, message.identity, message.ipAddressAndPort)) {
            auto stub = createStub(message.ipAddressAndPort);
            mStubs.emplace_back(std::move(stub));
            mTimestamps.emplace_back(TIMESTAMP_TIMEOUT);
            mTimestampTrials.emplace_back(TIMESTAMP_TRIALS);
            return true;
        }
    }

    stop();
    return false;
}

size_t TTBroadcasterDiscovery::getNeighborsCount() const {
    std::shared_lock neighborLock(mNeighborMutex);
    return mStubs.size();
}
