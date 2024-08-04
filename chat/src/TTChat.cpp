#include "TTChat.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <chrono>

TTChat::TTChat(TTChatSettings& settings, TTUtilsOutputStream& outputStream) :
        mPrimaryMessageQueue(settings.getPrimaryMessageQueue()),
        mSecondaryMessageQueue(settings.getSecondaryMessageQueue()),
        mStopped{false},
        mHeartbeatResult{},
        mWidth(settings.getTerminalWidth()),
        mHeight(settings.getTerminalHeight()),
        mSideWidth(mWidth * settings.getRatio()),
        mBlankLine(mWidth, ' '),
        mOutputStream(outputStream) {
    LOG_INFO("Constructing...");
    if (!mPrimaryMessageQueue->open()) {
        throw std::runtime_error("TTChat: Failed to open primary message queue!");
    }
    if (!mSecondaryMessageQueue->open()) {
        throw std::runtime_error("TTChat: Failed to open secondary message queue!");
    }
    // Set heartbeat sender thread
    std::promise<void> promise;
    mHeartbeatResult = promise.get_future();
    mHeartbeatThread = std::thread(&TTChat::heartbeat, this, std::move(promise));
    mHeartbeatThread.detach();
    LOG_INFO("Successfully constructed!");
}

TTChat::~TTChat() {
    LOG_INFO("Destructing...");
    mStopped.store(true);
    mHeartbeatResult.wait();
    LOG_INFO("Successfully destructed!");
}

void TTChat::run() {
    LOG_INFO("Started primary loop");
    if (!mPrimaryMessageQueue->alive() || !mSecondaryMessageQueue->alive()) {
        LOG_ERROR("Primary or secondary message queue is not alive!");
    } else {
        try {
            TTChatMessage message;
            while (true) {
                if (mStopped.load()) {
                    LOG_WARNING("Forced exit on primary loop");
                    break;
                }
                if (!mPrimaryMessageQueue->receive(reinterpret_cast<char*>(&message))) {
                    LOG_WARNING("Failed to send message!");
                    break;
                }
                if (!handle(message)) {
                    break;
                }
            }
        } catch (...) {
            LOG_ERROR("Caught unknown exception at primary loop!");
        }
    }
    mStopped.store(true);
    LOG_INFO("Completed primary loop");
}

void TTChat::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTChat::stopped() const {
    return mStopped.load();
}

void TTChat::heartbeat(std::promise<void> promise) {
    LOG_INFO("Started secondary (heartbeat) loop");
    if (!mPrimaryMessageQueue->alive() || !mSecondaryMessageQueue->alive()) {
        LOG_ERROR("Primary or secondary message queue is not alive!");
    } else {
        try {
            TTChatMessage message;
            message.setType(TTChatMessageType::HEARTBEAT); 
            while (true) {
                if (mStopped.load()) {
                    LOG_WARNING("Forced exit on secondary (heartbeat) loop");
                    break;
                }
                if (!mSecondaryMessageQueue->send(reinterpret_cast<const char*>(&message))) {
                    LOG_WARNING("Failed to send heartbeat message!");
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(TTCHAT_HEARTBEAT_TIMEOUT_MS));
            }
        } catch (...) {
            LOG_ERROR("Caught unknown exception at secondary (heartbeat) loop!");
        }
    }
    mStopped.store(true);
    promise.set_value();
    LOG_INFO("Completed secondary (heartbeat) loop");
}

bool TTChat::handle(const TTChatMessage& message) {
    switch (message.getType()) {
        case TTChatMessageType::CLEAR:
            LOG_INFO("Received clear message");
            mOutputStream.clear();
            return true;
        case TTChatMessageType::SEND:
            LOG_INFO("Received sender message");
            print(message);
            return true;
        case TTChatMessageType::RECEIVE:
            LOG_INFO("Received receiver message");
            print(message);
            return true;
        case TTChatMessageType::HEARTBEAT:
            LOG_INFO("Received heartbeat message");
            return true;
        case TTChatMessageType::GOODBYE:
            LOG_INFO("Received goodbye message");
            stop();
            return false;
        default:
            LOG_ERROR("Received unknown message");
            stop();
            return false;
    }
}

void TTChat::print(const TTChatMessage& message) {
    LOG_INFO("Formatting message to be printed...");
    // Remove spaces from the beggining and end
    std::string data = message.getData();
    const std::string delimiter = " ";
    std::string newData;
    for (auto charIterator = data.begin(); charIterator != data.end(); charIterator++) {
        if (*charIterator != delimiter.back()) {
            for (; charIterator != data.end(); charIterator++) {
                newData.push_back(*charIterator);
            }
            break;
        }
    }
    data = newData;
    if (data.back() == delimiter.back()) {
        data = data.substr(0, data.rfind(delimiter));
    }

    // Remove duplicated spaces
    newData.clear();
    for (auto charIterator = data.begin(); charIterator != data.end(); charIterator++) {
        if (*charIterator != delimiter.back()) {
            newData.push_back(*charIterator);
        } else {
            newData.push_back(delimiter.back());
            for (; charIterator != data.end() && *charIterator == delimiter.back(); charIterator++) {
            }
            if (charIterator != data.end()) {
                newData.push_back(*charIterator);
            } else {
                break;
            }
        }
    }
    data = newData;

    // Make sure message is not empty
    std::vector<std::string> lines;
    if (data.empty()) {
        lines.emplace_back(" ");
    } else {
        // Extract words from the message
        std::vector<std::string> words;
        size_t lastPosition = 0, currentPosition;
        while ((currentPosition = data.find(delimiter, lastPosition)) != std::string::npos) {
            std::string word = data.substr(lastPosition, currentPosition - lastPosition);
            lastPosition += (word.size() + 1);
            words.emplace_back(word);
        }
        words.emplace_back(data.substr(lastPosition, data.size()));

        // Create lines from the words
        auto wordIterator = words.begin();
        while (wordIterator != words.end()) {
            if (wordIterator->size() < mSideWidth) {
                std::string line;
                size_t leftSideWidth = mSideWidth;
                while (wordIterator != words.end() && line.size() + wordIterator->size() <= mSideWidth) {
                    line += *wordIterator;
                    line += delimiter;
                    ++wordIterator;
                }
                line.pop_back();
                lines.emplace_back(line);
            } else {
                lines.emplace_back(wordIterator->substr(0, mSideWidth));
                wordIterator->erase(0, mSideWidth);
            }
        }
    }

    // Create timestamp string
    const auto time = std::chrono::system_clock::to_time_t(message.getTimestamp());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
    const auto timestamp = ss.str();

    // Print message based on side
    LOG_INFO("Printing message...");
    if (message.getType() == TTChatMessageType::RECEIVE) {
        mOutputStream.print(timestamp).endl();
        for (auto &line : lines) {
            mOutputStream.print(line).endl();
        }
    } else {
        mOutputStream.print(mBlankLine.substr(0, mBlankLine.size() - timestamp.size()));
        mOutputStream.print(timestamp).endl();
        for (auto &line : lines) {
            mOutputStream.print(mBlankLine.substr(0, mBlankLine.size() - line.size()));
            mOutputStream.print(line).endl();
        }
    }
    mOutputStream.endl();
}
