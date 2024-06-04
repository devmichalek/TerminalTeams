#include "TTBroadcasterDiscovery.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterDiscovery::TTBroadcasterDiscovery(TTContactsHandler& contactsHandler,
                                               TTNetworkInterface interface,
                                               std::deque<std::string> neighbors) :
        mStopped{false}, mInterface(interface), mNeighbors(neighbors), mContactsHandler(contactsHandler) {

}

TTBroadcasterDiscovery::~TTBroadcasterDiscovery() {

}

void TTBroadcasterDiscovery::run() {
    LOG_INFO("Started broadcasting discovery");
    while (!stopped()) {

    }
    LOG_INFO("Stopped broadcasting discovery");
}

void TTBroadcasterDiscovery::stop() {
    LOG_INFO("Forced stop...");
    mStopped.store(true);
}

bool TTBroadcasterDiscovery::stopped() const {
    return mStopped.load();
}

bool TTBroadcasterDiscovery::handleGreet(const TTGreetMessage& message) {
    decltype(auto) id = mContactsHandler.get(message.identity);
    if (id == std::numeric_limits<size_t>::max()) {
        LOG_INFO("Handling greet, new contact id={}", message.identity);
        if (!mContactsHandler.create(message.nickname, message.identity, message.ipAddressAndPort)) {
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
    if (id == std::numeric_limits<size_t>::max()) {
        LOG_INFO("Handling heartbeat, ignoring not existing contact id={}", message.identity);
        return false;
    }
    LOG_INFO("Handling heartbeat, existing contact id={}", message.identity);

    return true;
}

std::string TTBroadcasterDiscovery::getNickname() {
    return mContactsHandler.get(0).nickname;
}

std::string TTBroadcasterDiscovery::getIdentity() {
    return mContactsHandler.get(0).identity;
}

std::string TTBroadcasterDiscovery::getIpAddressAndPort() {
    return mContactsHandler.get(0).ipAddressAndPort;
}
