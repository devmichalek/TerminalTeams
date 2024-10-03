#include "TTTextBox.hpp"
#include "TTUtilsSignals.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <list>
#include <fcntl.h>
#include <charconv>

TTTextBox::TTTextBox(const TTTextBoxSettings& settings,
    TTUtilsOutputStream& outputStream,
    TTUtilsInputStream& inputStream) :
        mPipe(settings.getNamedPipe()),
        mOutputStream(outputStream),
        mInputStream(inputStream) {
    LOG_INFO("Constructing...");
    // Create pipe
    if (!mPipe->create()) {
        throw std::runtime_error("TTTextBox: Failed to create named pipe!");
    }
    if (!mPipe->alive()) {
        throw std::runtime_error("TTTextBox: Failed to run, pipe is not alive!");
    }
    // Set main sender thread
    {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.block({ SIGPIPE });
        std::promise<void> mainPromise;
        mBlockers.push_back(mainPromise.get_future());
        mThreads.push_back(std::thread(&TTTextBox::main, this, std::move(mainPromise)));
        mThreads.back().detach();
        signals.unblock({ SIGPIPE });
    }
    // Set asynchronous reader thread
    {
        // Do not add it to the blockers as I/O may block forever
        mThreads.push_back(std::thread(&TTTextBox::asynchronousRead, this));
        mThreads.back().detach();
    }
    subscribeOnStop(mQueueCondition);
    subscribeOnStop(mWaitCondition);
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

void TTTextBox::wait() {
    std::unique_lock<std::mutex> lock(mWaitMutex);
    mWaitCondition.wait(lock, [this]() { return isStopped(); });
}

bool TTTextBox::parse(const std::string& line) {
    mOutputStream.clear();
    if (line.empty()) {
        LOG_WARNING("Received empty line from input!");
        return false;
    }

    if (line.front() == '#') {
        std::stringstream input{line.substr(1)};
        std::string arg;
        std::vector<std::string> args;
        while(std::getline(input, arg, ' ')) {
            args.push_back(arg);
        }
        return execute(args);
    }

    return send(line.c_str(), line.c_str() + line.size());
}

bool TTTextBox::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        LOG_WARNING("Received empty command from input!");
        return false;
    }

    const auto command = args.front();
    LOG_INFO("Executing \"{}\" command", command);
    if (command == "help") {
        if (args.size() > 1) {
            LOG_WARNING("Received \"{}\" command with invalid number of arguments!", command);
            return false;
        }
        mOutputStream.print("Type #help to print a help message").endl();
        mOutputStream.print("Type #quit to quit the application").endl();
        mOutputStream.print("Type #select <id> to select contact").endl();
        mOutputStream.print("Skip # and send a message to the currently selected contact.").endl();
        return true;
    }

    if (command == "quit") {
        if (args.size() > 1) {
            LOG_WARNING("Received \"{}\" command with invalid number of arguments!", command);
            return false;
        }
        stop();
        return true;
    }

    if (command == "select") {
        if (args.size() > 2) {
            LOG_WARNING("Received \"{}\" command with invalid number of arguments!", command);
            return false;
        }
        auto identity = args[1];
        if (!std::all_of(identity.begin(), identity.end(), ::isdigit)) {
            LOG_WARNING("Contacts selection attempt failed - string has characters other than digits!");
            return false;
        }

        if (identity.size() > TTTextBoxMessage::DATA_MAX_DIGITS) {
            LOG_WARNING("Contacts selection attempt failed - too many digits!");
            return false;
        }

        size_t id = 0;
        auto [ptr, ec] = std::from_chars(identity.c_str(), identity.c_str() + identity.size(), id);
        if (ec != std::errc()) {
            LOG_ERROR("Failed to convert string to decimal on contacts selection attempt!");
            return false;
        }
        auto message = std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::CONTACTS_SELECT, sizeof(id), reinterpret_cast<char*>(&id));
        queue(std::move(message));
        LOG_INFO("Successfully selected contact!");
        return true;
    }

    LOG_WARNING("Command \"{}\" not found!", command);
    return false;
}

bool TTTextBox::send(const char* cbegin, const char* cend) {
    LOG_INFO("Received casual message from the input");
    LOG_INFO("Splitting string into smaller chunks...");
    const long numOfMessages = (cend - cbegin) / TTTextBoxMessage::DATA_MAX_LENGTH;
    if (numOfMessages > 0) {
        for (auto i = numOfMessages; i > 0; --i) {
            auto message = std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::MESSAGE, TTTextBoxMessage::DATA_MAX_LENGTH, cbegin);
            queue(std::move(message));
            cbegin += TTTextBoxMessage::DATA_MAX_LENGTH;
        }
    }
    if (cbegin != cend) {
        auto message = std::make_unique<TTTextBoxMessage>(TTTextBoxStatus::MESSAGE, cend - cbegin, cbegin);
        queue(std::move(message));
    }
    LOG_INFO("Successfully splitted string into smaller chunks!");
    return true;
}

void TTTextBox::main(std::promise<void> promise) {
    LOG_INFO("Started textbox loop");
    try {
        while (!isStopped()) {
            LOG_INFO("Filling the list of messages...");
            std::list<std::unique_ptr<TTTextBoxMessage>> messages;
            {
                std::unique_lock<std::mutex> lock(mQueueMutex);
                auto waitTimeMs = std::chrono::milliseconds(TTTextBox::QUEUED_MSG_TIMEOUT_MS);
                mQueueCondition.wait_for(lock, waitTimeMs, [this]() {
                    return !mQueuedMessages.empty() || isStopped();
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
                if (isStopped()) {
                    break;
                }
                if (!mPipe->send(reinterpret_cast<char*>(message.get()))) {
                    LOG_ERROR("Failed to send message!");
                    throw std::runtime_error({});
                }
            }
            LOG_INFO("Successfully sent all messages!");
        }
    } catch (...) {
        LOG_ERROR("Caught unknown exception at textbox loop!");
    }
    sendGoodbye();
    stop();
    promise.set_value();
    LOG_INFO("Completed textbox loop");
}

void TTTextBox::queue(std::unique_ptr<TTTextBoxMessage> message) {
    {
        std::scoped_lock lock(mQueueMutex);
        mQueuedMessages.push(std::move(message));
    }
    mQueueCondition.notify_one();
}

void TTTextBox::sendGoodbye() {
    LOG_WARNING("Sending goodbye message...");
    TTTextBoxMessage message(TTTextBoxStatus::GOODBYE, 0, nullptr);
    mPipe->send(reinterpret_cast<const char*>(&message));
}

void TTTextBox::asynchronousRead() {
    mOutputStream.clear();
    mOutputStream.print("Type #help to print a help message").endl();
    while (!isStopped()) {
        std::string line;
        mInputStream.readline(line);
        if (!parse(line)) {
            LOG_WARNING("Failed to parse input!");
            continue;
        }
    }
}
