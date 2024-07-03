#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler) :
        mStopped{false}, mContactsHandler(contactsHandler), mChatHandler(chatHandler) {
    LOG_INFO("Constructing...");
    LOG_INFO("Successfully constructed!");
}

TTBroadcasterChat::~TTBroadcasterChat() {
    LOG_INFO("Destructing...");
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
    return true;
}

bool TTBroadcasterChat::handleNarrate(const TTNarrateMessages& messages) {
    return true;
}

const std::string& TTBroadcasterChat::getIdentity() const {
    decltype(auto) opt = mContactsHandler.get(0);
    if (opt == std::nullopt) {
        throw std::runtime_error("TTBroadcasterDiscovery: Failed to get identity!");
    }
    return opt->identity;
}

