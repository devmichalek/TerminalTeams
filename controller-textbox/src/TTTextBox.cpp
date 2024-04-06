#include "TTTextBox.hpp"
#include <iostream>
#include <list>
#include <fcntl.h>
#include <charconv>

TTTextBox::TTTextBox(const TTTextBoxSettings& settings) :
    mNamedPipeDescriptor(-1),
    mStopped{false}
{
    const std::string classNamePrefix = "TTTextBox: ";
    errno = 0;
    for (auto attempt = TTTEXTBOX_NAMED_PIPE_READY_TRY_COUNT; attempt > 0; --attempt) {
        if (stopped()) {
            return; // Forced exit
        }
        const auto pipePath = TTTextBoxSettings::getPipePath(settings.getUniqueName());
        if ((mNamedPipeDescriptor = open(pipePath.c_str(), O_WRONLY)) != -1) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_NAMED_PIPE_READY_TIMEOUT_MS));
    }

    if (mNamedPipeDescriptor == -1) {
        throw std::runtime_error(classNamePrefix + "Failed to open named pipe, errno=" + std::to_string(errno));
    }

    // Set main sender thread
    std::promise<void> mainPromise;
    mBlockers.push_back(mainPromise.get_future());
    mThreads.push_back(std::thread(&TTTextBox::main, this, std::move(mainPromise)));
    mThreads.back().detach();
}

TTTextBox::~TTTextBox() {
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    close(mNamedPipeDescriptor);
}

void TTTextBox::run() {
    if (mNamedPipeDescriptor == -1) {
        return;
    }
    try {
        std::cout << "Type #<Contact ID> to switch contact." << std::endl;
        std::cout << "Skip # to send a normal message." << std::endl << std::endl;
        while (!stopped()) {
            std::string line;
            std::getline(std::cin, line);
            if (line.empty()) {
                continue;
            }
            send(line.c_str(), line.c_str() + line.size());
        }
    } catch (...) {
        // ...
    }
    stop();
}

void TTTextBox::stop() {
    mStopped.store(true);
}

bool TTTextBox::stopped() const {
    return mStopped.load();
}

void TTTextBox::main(std::promise<void> promise) {
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
                while (!mQueuedMessages.empty()) {
                    messages.push_back(std::move(mQueuedMessages.front()));
                    mQueuedMessages.pop();
                }
                // Do not remove scope guards, risk of deadlock
            }
            // If there are no new messages insert heartbeat message
            if (messages.empty()) {
                messages.push_back(std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::HEARTBEAT, 0, nullptr));
            }
            // Queue all messages
            for (auto &message : messages) {
                if (write(mNamedPipeDescriptor, message.get(), sizeof(TTTextBoxMessage)) < 0) {
                    throw std::runtime_error({});
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_SEND_TIMEOUT_MS));
        }
    } catch (...) {
        // ...
    }
    stop();
    promise.set_value();
}

void TTTextBox::send(const char* cbegin, const char* cend) {
    if (*cbegin == '#' && (cbegin + 1 < cend)) {
        if (std::all_of(cbegin + 1, cend, ::isdigit)) {
            if ((cend - cbegin - 1) <= TTTEXTBOX_DATA_MAX_DIGITS) {
                {
                    size_t id = 0;
                    auto [ptr, ec] = std::from_chars(cbegin + 1, cend, id);
                    if (ec != std::errc()) {
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
                std::cout << "\033[2J\033[1;1H" << std::flush;
                return;
            }
        }
    }
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
    std::cout << "\033[2J\033[1;1H" << std::flush;
}
