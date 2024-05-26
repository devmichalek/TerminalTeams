#include "TTContacts.hpp"

TTContacts::TTContacts(const TTContactsSettings& settings, const TTUtilsOutputStream& outputStream) :
        mOutputStream(outputStream),
        mSharedMem(std::move(settings.getSharedMemory())),
        mTerminalWidth(settings.getTerminalWidth()),
        mTerminalHeight(settings.getTerminalHeight()) {
    LOG_INFO("Constructing...");
    if (!mSharedMem->open()) {
        throw std::runtime_error("TTContacts: Failed to open shared memory!");
    } else {
        LOG_INFO("Successfully opened shared memory!");
    }
}

TTContacts::~TTContacts() {
    LOG_INFO("Destructing...");
}

void TTContacts::run() {
    LOG_INFO("Started contacts loop");
    while (!stopped() && mSharedMem->alive()) {
        TTContactsMessage newMessage;
        if (!mSharedMem->receive(reinterpret_cast<void*>(&newMessage))) {
            LOG_WARNING("Failed to receive message!");
            break;
        }
        if (handle(newMessage)) {
            refresh();
        }
    }
    LOG_INFO("Completed contacts loop");
}

void TTContacts::stop() {
    LOG_INFO("Forced stop...");
    mStopped.store(true);
}

bool TTContacts::stopped() const {
    return mStopped.load();
}

size_t TTContacts::size() const {
    return mContacts.size();
}

bool TTContacts::handle(const TTContactsMessage& message) {
    if (message.status == TTContactsStatus::HEARTBEAT) {
        LOG_INFO("Received heartbeat message");
        return false;
    } else {
        if (message.status == TTContactsStatus::ACTIVE && message.id >= mContacts.size()) {
            std::string nickname(message.data, message.data + message.dataLength);
            auto newContact = std::make_tuple(message.id, nickname, message.status);
            mContacts.push_back(newContact);
            LOG_INFO("Received new contact message id={}, nickname={}, status={}", message.id, nickname, (size_t)message.status);
        } else {
            auto& contact = mContacts[message.id];
            std::get<2>(contact) = message.status;
            LOG_INFO("Received update contact message id={}, status={}", message.id, (size_t)message.status);
        }
    }
    return true;
}

void TTContacts::refresh() {
    LOG_INFO("Started refreshing window, number of contacts={}", size());
    mOutputStream.clear();
    const std::array<std::string, 8> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
    for (auto &contact : mContacts) {
        mOutputStream.print("#").print(std::to_string(std::get<0>(contact)));
        mOutputStream.print(" ").print(std::get<1>(contact));
        mOutputStream.print(" ").print(statuses[std::get<2>(contact)]);
        mOutputStream.endl();
    }
}
