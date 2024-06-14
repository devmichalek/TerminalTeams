#include "TTBroadcasterChat.hpp"
#include "TTDiagnosticsLogger.hpp"

TTBroadcasterChat::TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler) :
        mStopped{false}, mContactsHandler(contactsHandler), mChatHandler(chatHandler) {

}

TTBroadcasterChat::~TTBroadcasterChat() {
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
    return mContactsHandler.get(0).identity;
}

