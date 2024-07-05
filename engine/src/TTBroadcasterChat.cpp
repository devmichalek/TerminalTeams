#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler) :
        mStopped{false}, mContactsHandler(contactsHandler), mChatHandler(chatHandler) {
    LOG_INFO("Constructing...");
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

bool TTBroadcasterChat::handleTell(const TTNarrateMessage& message) {
    LOG_INFO("Handling tell...");
    return true;
}

bool TTBroadcasterChat::handleNarrate(const TTNarrateMessages& messages) {
    LOG_INFO("Handling narrate...");
    return true;
}

std::string TTBroadcasterChat::getIdentity() const {
    auto opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get identity!");
    }
    return opt->identity;
}

