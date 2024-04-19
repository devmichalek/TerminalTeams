#include "TTContacts.hpp"

TTContacts::TTContacts(const TTContactsSettings& settings,
    TTContactsCallbackQuit callbackQuit,
    const TTContactsOutputStream& outputStream) :
        mConsumer(std::move(settings.getConsumer())),
        mCallbackQuit(callbackQuit),
        mOutputStream(outputStream),
        mTerminalWidth(settings.getTerminalWidth()),
        mTerminalHeight(settings.getTerminalHeight()) {
    const std::string classNamePrefix = "TTContacts: ";
    if (!mConsumer->init()) {
        throw std::runtime_error(classNamePrefix + "Failed to initialize consumer!");
    }
}

void TTContacts::run() {
    try {
        while (!mCallbackQuit() && mConsumer->alive()) {
            auto newMessage = mConsumer->get();
            if (!newMessage) {
                break;
            }
            if (handle(*(newMessage.get()))) {
                refresh();
            }
        }
    } catch (...) {
        // ...
    }
}

size_t TTContacts::size() const {
    return mContacts.size();
}

bool TTContacts::handle(const TTContactsMessage& message) {
    if (message.status == TTContactsStatus::HEARTBEAT) {
        // Nothing to be done
        return false;
    } else {
        if (message.status == TTContactsStatus::ACTIVE && message.id >= mContacts.size()) {
            std::string nickname(message.data, message.data + message.dataLength);
            auto newContact = std::make_tuple(message.id, nickname, message.status);
            mContacts.push_back(newContact);
        } else {
            auto& contact = mContacts[message.id];
            std::get<2>(contact) = message.status;
        }
    }
    return true;
}

void TTContacts::refresh() {
    mOutputStream.print("\033[2J\033[1;1H").flush(); // Clear window
    const std::array<std::string, 8> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
    for (auto &contact : mContacts) {
        mOutputStream.print("#").print(std::to_string(std::get<0>(contact)));
        mOutputStream.print(" ").print(std::get<1>(contact));
        mOutputStream.print(" ").print(statuses[std::get<2>(contact)]);
        mOutputStream.endl();
    }
}
