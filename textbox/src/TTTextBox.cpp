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
    LOG_INFO("Constructing...");
    // Open pipe
    if (!mPipe->open()) {
        throw std::runtime_error("TTTextBox: Failed to open named pipe!");
    }
    // Set main sender thread
    std::promise<void> mainPromise;
    mBlockers.push_back(mainPromise.get_future());
    mThreads.push_back(std::thread(&TTTextBox::main, this, std::move(mainPromise)));
    mThreads.back().detach();
    LOG_INFO("Successfully constructed!");
}

TTTextBox::~TTTextBox() {
    LOG_INFO("Destructing...");
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    LOG_INFO("Successfully destructed!");
}

void TTTextBox::run() {
    if (!mPipe->alive()) {
        LOG_ERROR("Failed to run, pipe is not open!");
        return;
    }
    try {
        mOutputStream.print("Type #<Contact ID> to switch contact.").endl();
        mOutputStream.print("Skip # to send a normal message.").endl().endl();
        while (!stopped()) {
            std::string line;
            std::getline(std::cin, line);
            if (line.empty()) {
                LOG_WARNING("Received empty line from input!");
                break;
            }
            LOG_INFO("Received next line from the input");
            send(line.c_str(), line.c_str() + line.size());
        }
    } catch (...) {
        LOG_ERROR("Caught unknown exception!");
    }
    stop();
}

void TTTextBox::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTTextBox::stopped() const {
    return mStopped.load();
}

void TTTextBox::main(std::promise<void> promise) {
    LOG_INFO("Started textbox loop");
    if (mPipe->alive()) {
        try {
            while (!stopped()) {
                LOG_INFO("Filling the list of messages...");
                std::list<std::unique_ptr<TTTextBoxMessage>> messages;
                {
                    std::unique_lock<std::mutex> lock(mQueueMutex);
                    auto waitTimeMs = std::chrono::milliseconds(TTTEXTBOX_QUEUED_MSG_TIMEOUT_MS);
                    mQueueCondition.wait_for(lock, waitTimeMs, [this]() {
                        return !mQueuedMessages.empty();
                    });
                    if (!mQueuedMessages.empty()) {
                        size_t counter = 0;
                        while (!mQueuedMessages.empty()) {
                            ++counter;
                            messages.push_back(std::move(mQueuedMessages.front()));
                            mQueuedMessages.pop();
                        }
                        LOG_INFO("Inserted {} messages...", counter);
                    }
                    // Do not remove scope guards, risk of deadlock
                }
                // If there are no new messages insert heartbeat message
                if (messages.empty()) {
                    LOG_INFO("Inserting heartbeat message...");
                    messages.push_back(std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::HEARTBEAT, 0, nullptr));
                }
                LOG_INFO("Sending all messages...");
                for (auto &message : messages) {
                    if (!mPipe->send(reinterpret_cast<char*>(message.get()))) {
                        LOG_ERROR("Failed to send message!");
                        throw std::runtime_error({});
                    }
                }
                LOG_INFO("Successfully sent all messages!");
                std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_SEND_TIMEOUT_MS));
            }
        } catch (...) {
            LOG_ERROR("Caught unknown exception at textbox loop!");
        }
    }
    stop();
    promise.set_value();
    LOG_INFO("Completed textbox loop");
}

void TTTextBox::send(const char* cbegin, const char* cend) {
    if (*cbegin == '#' && (cbegin + 1 < cend)) {
        LOG_INFO("Converting string to decimal...");
        if (std::all_of(cbegin + 1, cend, ::isdigit)) {
            if ((cend - cbegin - 1) <= TTTEXTBOX_DATA_MAX_DIGITS) {
                {
                    size_t id = 0;
                    auto [ptr, ec] = std::from_chars(cbegin + 1, cend, id);
                    if (ec != std::errc()) {
                        LOG_ERROR("Failed to convert string to decimal!");
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
                LOG_INFO("Successfully converted string to decimal!");
                return;
            }
        }
    }
    LOG_INFO("Splitting string into smaller chunks...");
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
    LOG_INFO("Successfully splitted string into smaller chunks!");
}
