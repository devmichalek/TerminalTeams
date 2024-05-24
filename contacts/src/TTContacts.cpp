#include "TTContacts.hpp"

TTContacts::TTContacts(const TTContactsSettings& settings, const TTUtilsOutputStream& outputStream) :
        mOutputStream(outputStream),
        mSharedMem(std::move(settings.getSharedMemory())),
        mTerminalWidth(settings.getTerminalWidth()),
        mTerminalHeight(settings.getTerminalHeight()) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Constructing...", mClassNamePrefix);
    if (!mSharedMem->open()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to open shared memory!");
    } else {
        logger.info("{} Successfully opened shared memory!", mClassNamePrefix);
    }
}

TTContacts::~TTContacts() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
}

void TTContacts::run() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started contacts loop", mClassNamePrefix);
    while (!stopped() && mSharedMem->alive()) {
        TTContactsMessage newMessage;
        if (!mSharedMem->receive(reinterpret_cast<void*>(&newMessage))) {
            logger.warning("{} Failed to receive message!", mClassNamePrefix);
            break;
        }
        if (handle(newMessage)) {
            refresh();
        }
    }
    logger.info("{} Completed contacts loop", mClassNamePrefix);
}

void TTContacts::stop() {
    TTDiagnosticsLogger::getInstance().info("{} Forced stop...", mClassNamePrefix);
    mStopped.store(true);
}

bool TTContacts::stopped() const {
    return mStopped.load();
}

size_t TTContacts::size() const {
    return mContacts.size();
}

bool TTContacts::handle(const TTContactsMessage& message) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (message.status == TTContactsStatus::HEARTBEAT) {
        logger.info("{} Received heartbeat message", mClassNamePrefix);
        return false;
    } else {
        if (message.status == TTContactsStatus::ACTIVE && message.id >= mContacts.size()) {
            std::string nickname(message.data, message.data + message.dataLength);
            auto newContact = std::make_tuple(message.id, nickname, message.status);
            mContacts.push_back(newContact);
            logger.info("{} Received new contact message id={}, nickname={}, status={}", mClassNamePrefix, message.id, nickname, (size_t)message.status);
        } else {
            auto& contact = mContacts[message.id];
            std::get<2>(contact) = message.status;
            logger.info("{} Received update contact message id={}, status={}", mClassNamePrefix, message.id, (size_t)message.status);
        }
    }
    return true;
}

void TTContacts::refresh() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started refreshing window, number of contacts={}", mClassNamePrefix, size());
    mOutputStream.clear();
    const std::array<std::string, 8> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
    for (auto &contact : mContacts) {
        mOutputStream.print("#").print(std::to_string(std::get<0>(contact)));
        mOutputStream.print(" ").print(std::get<1>(contact));
        mOutputStream.print(" ").print(statuses[std::get<2>(contact)]);
        mOutputStream.endl();
    }
}
