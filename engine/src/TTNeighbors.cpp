#include "TTNeighbors.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighbors::TTNeighbors(TTNetworkInterface interface, std::deque<std::string> neighbors, TTContactsHandler& contactsHandler, TTChatHandler& chatHandler) :
        mInterface(interface), mNeighbors(neighbors), mContactsHandler(contactsHandler), mChatHandler(chatHandler) {

}

TTNeighbors::~TTNeighbors() {

}

bool TTNeighbors::handleTell(const TTNarrateMessage& message) {
    return true;
}

bool TTNeighbors::handleNarrate(const TTNarrateMessages& messages) {
    return true;
}

bool TTNeighbors::handleGreet(const TTGreetMessage& message) {
    return true;
}

bool TTNeighbors::handleHeartbeat(const TTHeartbeatMessage& message) {
    return true;
}

std::string TTNeighbors::getNickname() {
    return mContactsHandler.get(0).nickname;
}

std::string TTNeighbors::getIdentity() {
    return mContactsHandler.get(0).identity;
}

std::string TTNeighbors::getIpAddressAndPort() {
    return mContactsHandler.get(0).ipAddressAndPort;
}
