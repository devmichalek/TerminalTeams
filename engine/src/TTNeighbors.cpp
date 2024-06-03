#include "TTNeighbors.hpp"
#include "TTDiagnosticsLogger.hpp"

TTNeighbors(TTInterface interface, TTContactsHandler& contactsHandler, TTChatHandler& chatHandler) :
        mInterface(interface), mContactsHandler(contactsHandler), mChatHandler(chatHandler) {

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
    return contactsHandler.get(0).nickname;
}

std::string TTNeighbors::getIdentity() {
    return contactsHandler.get(0).identity;
}

std::string TTNeighbors::getIpAddressAndPort() {
    return contactsHandler.get(0).ipAddressAndPort;
}
