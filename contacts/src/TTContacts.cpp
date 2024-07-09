#include "TTContacts.hpp"

TTContacts::TTContacts(const TTContactsSettings& settings, const TTUtilsOutputStream& outputStream) :
        mOutputStream(outputStream),
        mSharedMem(std::move(settings.getSharedMemory())),
        mTerminalWidth(settings.getTerminalWidth()),
        mTerminalHeight(settings.getTerminalHeight()) {
    LOG_INFO("Constructing...");
    if (!mSharedMem->open()) {
        throw std::runtime_error("TTContacts: Failed to open shared memory!");
    }
    LOG_INFO("Successfully constructed!");
}

TTContacts::~TTContacts() {
    LOG_INFO("Successfully destructed!");
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
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTContacts::stopped() const {
    return mStopped.load();
}

bool TTContacts::handle(const TTContactsMessage& message) {
    switch (message.getStatus()) {
        case TTContactsStatus::STATE:
            if (message.getState() == TTContactsState::ACTIVE && message.getIdentity() >= mEntries.size()) {
                mEntries.emplace_back(message.getIdentity(), message.getState(), message.getNickname());
                LOG_INFO("Received new contact message id={}, nickname={}, state={}", message.getIdentity(), message.getNickname(), (size_t)message.getState());
            } else {
                mEntries[message.getIdentity()].state = message.getState();
                LOG_INFO("Received update contact message id={}, state={}", message.getIdentity(), (size_t)message.getState());
            }
            return true;
        case TTContactsStatus::HEARTBEAT:
            LOG_INFO("Received heartbeat message");
            return false;
        case TTContactsStatus::GOODBYE:
            LOG_INFO("Received goodbye message");
            stop();
            return false;
        default:
            LOG_INFO("Received unknown message");
            stop();
            return false;
    }
}

void TTContacts::refresh() {
    LOG_INFO("Started refreshing window, number of entries={}", mEntries.size());
    mOutputStream.clear();
    const std::array<std::string, 8> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
    for (auto &entry : mEntries) {
        mOutputStream.print("#").print(std::to_string(entry.identity));
        mOutputStream.print(" ").print(entry.nickname);
        mOutputStream.print(" ").print(statuses[static_cast<size_t>(entry.state)]);
        mOutputStream.endl();
    }
}
