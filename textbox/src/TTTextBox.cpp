#include "TTTextBox.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <iostream>
#include <list>
#include <fcntl.h>
#include <charconv>

TTTextBox::TTTextBox(const TTTextBoxSettings& settings, const TTUtilsOutputStream& outputStream) :
        mPipe(settings.getNamedPipe()),
        mOutputStream(outputStream),
        mStopped{false} {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Constructing...", mClassNamePrefix);
    // Open pipe
    if (!mPipe->open()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to open named pipe!");
    } else {
        logger.info("{} Successfully opened named pipe!", mClassNamePrefix);
    }
    // Set main sender thread
    std::promise<void> mainPromise;
    mBlockers.push_back(mainPromise.get_future());
    mThreads.push_back(std::thread(&TTTextBox::main, this, std::move(mainPromise)));
    mThreads.back().detach();
}

TTTextBox::~TTTextBox() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
}

void TTTextBox::run() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (!mPipe->alive()) {
        logger.error("{} Failed to run, pipe is not open!", mClassNamePrefix);
        return;
    }
    try {
        mOutputStream.print("Type #<Contact ID> to switch contact.").endl();
        mOutputStream.print("Skip # to send a normal message.").endl().endl();
        while (!stopped()) {
            std::string line;
            std::getline(std::cin, line);
            if (line.empty()) {
                logger.warning("{} Received empty line from input!", mClassNamePrefix);
                break;
            }
            logger.info("{} Received next line from input", mClassNamePrefix);
            send(line.c_str(), line.c_str() + line.size());
        }
    } catch (...) {
        logger.error("{} Caught unknown exception!", mClassNamePrefix);
    }
    stop();
}

void TTTextBox::stop() {
    TTDiagnosticsLogger::getInstance().info("{} Forced stop...", mClassNamePrefix);
    mStopped.store(true);
}

bool TTTextBox::stopped() const {
    return mStopped.load();
}

void TTTextBox::main(std::promise<void> promise) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started textbox loop", mClassNamePrefix);
    if (mPipe->alive()) {
        try {
            while (!stopped()) {
                // Fill the list of messages
                std::list<std::unique_ptr<TTTextBoxMessage>> messages;
                {
                    std::unique_lock<std::mutex> lock(mQueueMutex);
                    auto waitTimeMs = std::chrono::milliseconds(TTTEXTBOX_QUEUED_MSG_TIMEOUT_MS);
                    mQueueCondition.wait_for(lock, waitTimeMs, [this]() {
                        return !mQueuedMessages.empty();
                    });
                    if (!mQueuedMessages.empty()) {
                        logger.info("{} Inserting messages...", mClassNamePrefix);
                        while (!mQueuedMessages.empty()) {
                            messages.push_back(std::move(mQueuedMessages.front()));
                            mQueuedMessages.pop();
                        }
                    }
                    // Do not remove scope guards, risk of deadlock
                }
                // If there are no new messages insert heartbeat message
                if (messages.empty()) {
                    logger.info("{} Inserting heartbeat message...", mClassNamePrefix);
                    messages.push_back(std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::HEARTBEAT, 0, nullptr));
                }
                // Queue all messages
                for (auto &message : messages) {
                    if (!mPipe->send(message.get())) {
                        logger.error("{} Failed to send message!", mClassNamePrefix);
                        throw std::runtime_error({});
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_SEND_TIMEOUT_MS));
            }
        } catch (...) {
            logger.error("{} Caught unknown exception at textbox loop!", mClassNamePrefix);
        }
        
    }
    stop();
    promise.set_value();
    logger.info("{} Completed textbox loop", mClassNamePrefix);
}

void TTTextBox::send(const char* cbegin, const char* cend) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    if (*cbegin == '#' && (cbegin + 1 < cend)) {
        logger.info("{} Converting string to decimal...", mClassNamePrefix);
        if (std::all_of(cbegin + 1, cend, ::isdigit)) {
            if ((cend - cbegin - 1) <= TTTEXTBOX_DATA_MAX_DIGITS) {
                {
                    size_t id = 0;
                    auto [ptr, ec] = std::from_chars(cbegin + 1, cend, id);
                    if (ec != std::errc()) {
                        logger.error("{} Failed to convert string to decimal!", mClassNamePrefix);
                        throw std::runtime_error({});
                    }
                    std::scoped_lock<std::mutex> lock(mQueueMutex);
                    mQueuedMessages.push(
                        std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::CONTACTS_SWITCH,
                            sizeof(id),
                            reinterpret_cast<char*>(&id))
                    );
                }
                mQueueCondition.notify_one();
                mOutputStream.clear();
                logger.info("{} Successfully converted string to decimal!", mClassNamePrefix);
                return;
            }
        }
    }
    logger.info("{} Splitting string into smaller chunks...", mClassNamePrefix);
    const long numOfMessages = (cend - cbegin) / TTTEXTBOX_DATA_MAX_LENGTH;
    if (numOfMessages > 0) {
        for (auto i = numOfMessages; i > 0; --i) {
            {
                std::scoped_lock<std::mutex> lock(mQueueMutex);
                mQueuedMessages.push(
                    std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::MESSAGE,
                        TTTEXTBOX_DATA_MAX_LENGTH,
                        cbegin)
                );
            }
            mQueueCondition.notify_one();
            cbegin += TTTEXTBOX_DATA_MAX_LENGTH;
        }
    }
    if (cbegin != cend) {
        {
            std::scoped_lock<std::mutex> lock(mQueueMutex);
            mQueuedMessages.push(
                std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::MESSAGE,
                    cend - cbegin,
                    cbegin)
            );
        }
        mQueueCondition.notify_one();
    }
    mOutputStream.clear();
    logger.info("{} Successfully splitted string into smaller chunks!", mClassNamePrefix);

}
