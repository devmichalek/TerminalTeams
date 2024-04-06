#include "TTTextBox.hpp"
#include <iostream>
#include <list>
#include <fcntl.h>

TTTextBox::TTTextBox(const TTTextBoxSettings& settings) :
    mNamedPipeDescriptor(-1),
    mSocketDescriptor(-1)
{
    const std::string classNamePrefix = "TTTextBox: ";
    errno = 0;
    for (auto attempt = TTTEXTBOX_NAMED_PIPE_READY_TRY_COUNT; attempt > 0; --attempt) {
        if (mStopped.load()) {
            return; // Forced exit
        }
        const auto pipePath = TTTextBoxSettings::getPipePath(settings.getUniqueName());
        if ((mNamedPipeDescriptor = open(pipePath.c_str(), O_RDONLY)) != -1) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_NAMED_PIPE_READY_TIMEOUT_MS));
    }

    if (mNamedPipeDescriptor == -1) {
        throw std::runtime_error(classNamePrefix + "Failed to open named pipe, errno=" + std::to_string(errno));
    }

    // todo: remove later
    mSocketDescriptor = 0;
}

TTTextBox::~TTTextBox() {
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    close(mNamedPipeDescriptor);
}

void TTTextBox::run() {
    if (mNamedPipeDescriptor == -1 || mSocketDescriptor == -1) {
        return;
    }
    try {
        std::cout << "Type #<Contact ID> to switch contact." << std::endl;
        std::cout << "Skip # to send a normal message." << std::endl << std::endl;
        while (true) {
            // Read input
            std::string line;
            std::getline(std::cin, line);
            if (mStopped.load()) {
                break; // Forced exit
            }
            // todo: check for TTTEXTBOX_DATA_MAX_LENGTH
            // Clear the window
            std::cout << "\033[2J\033[1;1H" << std::flush;
        }
    } catch (...) {
        // ...
    }
    stop();
}

void TTTextBox::stop() {
    mStopped.store(true);
}

void TTTextBox::heartbeat(std::promise<void> promise) {
    try {
        while (!mStopped.load()) {
            // ...
        }
    } catch (...) {
        // ...
    }
    stop();
    promise.set_value();
}

void TTTextBox::main(std::promise<void> promise) {
    try {
        while (!mStopped.load()) {
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
