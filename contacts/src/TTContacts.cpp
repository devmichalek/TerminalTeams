#include "TTContacts.hpp"

TTContacts::TTContacts(const TTContactsSettings& settings,
    TTContactsCallbackQuit callbackQuit,
    const TTContactsOutputStream& outputStream) :
        mConsumer(std::move(settings.getConsumer())),
        mCallbackQuit(callbackQuit),
        mOutputStream(outputStream),
        mLogger(TTDiagnosticsLogger::getInstance()),
        mTerminalWidth(settings.getTerminalWidth()),
        mTerminalHeight(settings.getTerminalHeight()) {
    mLogger.info("{} Constructing...", mClassNamePrefix);
    if (!mConsumer->init()) {
        mLogger.error("{} Failed to initialize consumer!", mClassNamePrefix);
        throw std::runtime_error(mClassNamePrefix + "Failed to initialize consumer!");
    } else {
        mLogger.info("{} Successfully initialized consumer!", mClassNamePrefix);
    }
}

void TTContacts::run() {
    mLogger.info("{} Started contacts loop", mClassNamePrefix);
    while (!mCallbackQuit() && mConsumer->alive()) {
        auto newMessage = mConsumer->get();
        if (!newMessage) {
            mLogger.warning("{} Received null message!", mClassNamePrefix);
            break;
        }
        if (handle(*(newMessage.get()))) {
            refresh();
        }
    }
    mLogger.info("{} Completed contacts loop", mClassNamePrefix);
}

size_t TTContacts::size() const {
    return mContacts.size();
}

bool TTContacts::handle(const TTContactsMessage& message) {
    if (message.status == TTContactsStatus::HEARTBEAT) {
        mLogger.info("{} Received heartbeat message", mClassNamePrefix);
        return false;
    } else {
        if (message.status == TTContactsStatus::ACTIVE && message.id >= mContacts.size()) {
            std::string nickname(message.data, message.data + message.dataLength);
            auto newContact = std::make_tuple(message.id, nickname, message.status);
            mContacts.push_back(newContact);
            mLogger.info("{} Received new contact message id={}, nickname={}, status={}", mClassNamePrefix, message.id, nickname, (size_t)message.status);
        } else {
            auto& contact = mContacts[message.id];
            std::get<2>(contact) = message.status;
            mLogger.info("{} Received update contact message id={}, status={}", mClassNamePrefix, message.id, (size_t)message.status);
        }
    }
    return true;
}

void TTContacts::refresh() {
    mLogger.info("{} Started refreshing window, number of contacts={}", mClassNamePrefix, size());
    mOutputStream.print("\033[2J\033[1;1H").flush(); // Clear window
    const std::array<std::string, 8> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
    for (auto &contact : mContacts) {
        mOutputStream.print("#").print(std::to_string(std::get<0>(contact)));
        mOutputStream.print(" ").print(std::get<1>(contact));
        mOutputStream.print(" ").print(statuses[std::get<2>(contact)]);
        mOutputStream.endl();
    }
}
