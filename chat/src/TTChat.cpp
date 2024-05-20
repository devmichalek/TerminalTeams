#include "TTChat.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <chrono>

TTChat::TTChat(const TTChatSettings& settings, const TTUtilsOutputStream& outputStream) :
        mPrimaryMessageQueue(settings.getPrimaryMessageQueue()),
        mSecondaryMessageQueue(settings.getSecondaryMessageQueue()),
        mForcedQuit{false},
        mHeartbeatResult{},
        mWidth(settings.getTerminalWidth()),
        mHeight(settings.getTerminalHeight()),
        mSideWidth(mWidth * settings.getRatio()),
        mBlankLine(mWidth, ' '),
        mOutputStream(outputStream) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Constructing...", mClassNamePrefix);
    if (!mPrimaryMessageQueue->open()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to open primary message queue!");
    } else {
        logger.info("{} Successfully opened primary message queue!", mClassNamePrefix);
    }
    if (!mSecondaryMessageQueue->open()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to open secondary message queue!");
    } else {
        logger.info("{} Successfully opened secondary message queue!", mClassNamePrefix);
    }
    // Set heartbeat sender thread
    std::promise<void> promise;
    mHeartbeatResult = promise.get_future();
    mHeartbeatThread = std::thread(&TTChat::heartbeat, this, std::move(promise));
    mHeartbeatThread.detach();
}

TTChat::~TTChat() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    mForcedQuit.store(true);
    mHeartbeatResult.wait();
}

void TTChat::run() {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started primary loop", mClassNamePrefix);
    if (mPrimaryMessageQueue->alive() && mSecondaryMessageQueue->alive()) {
        try {
            TTChatMessage message;
            while (true) {
                if (mForcedQuit.load()) {
                    logger.warning("{} Forced exit on primary loop", mClassNamePrefix);
                    break;
                }
                if (!mPrimaryMessageQueue->receive(reinterpret_cast<char*>(&message))) {
                    logger.warning("{} Failed to send message!", mClassNamePrefix);
                    break;
                }
                message.data[message.dataLength] = '\0';
                handle(message);
            }
        } catch (...) {
            logger.error("{} Caught unknown exception at primary loop!", mClassNamePrefix);
        }
    }
    mForcedQuit.store(true);
    logger.info("{} Completed primary loop", mClassNamePrefix);
}

void TTChat::stop() {
    TTDiagnosticsLogger::getInstance().info("{} Forced stop...", mClassNamePrefix);
    mStopped.store(true);
}

bool TTChat::stopped() const {
    return mStopped.load();
}

void TTChat::heartbeat(std::promise<void> promise) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started secondary (heartbeat) loop", mClassNamePrefix);
    if (mPrimaryMessageQueue->alive() && mSecondaryMessageQueue->alive()) {
        try {
            char dummyBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
            while (true) {
                if (mForcedQuit.load()) {
                    logger.warning("{} Forced exit on secondary (heartbeat) loop", mClassNamePrefix);
                    break;
                }
                if (!mSecondaryMessageQueue->send(reinterpret_cast<const char*>(&dummyBuffer[0]))) {
                    logger.warning("{} Failed to send heartbeat message!", mClassNamePrefix);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(TTCHAT_HEARTBEAT_TIMEOUT_MS));
            }
        } catch (...) {
            logger.error("{} Caught unknown exception at secondary (heartbeat) loop!", mClassNamePrefix);
        }
    }
    mForcedQuit.store(true);
    promise.set_value();
    logger.info("{} Completed secondary (heartbeat) loop", mClassNamePrefix);
}

void TTChat::handle(const TTChatMessage& message) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    switch (message.type) {
        case TTChatMessageType::CLEAR:
            logger.info("{} Received clear message", mClassNamePrefix);
            mOutputStream.clear();
            break;
        case TTChatMessageType::SEND:
            logger.info("{} Received sender message", mClassNamePrefix);
            print(message.data, message.timestamp, false);
            break;
        case TTChatMessageType::RECEIVE:
            logger.info("{} Received receiver message", mClassNamePrefix);
            print(message.data, message.timestamp, true);
            break;
        case TTChatMessageType::HEARTBEAT:
            logger.info("{} Received heartbeat message", mClassNamePrefix);
        default:
            break;
    }
}

void TTChat::print(const char* cmessage, TTChatTimestamp timestmap, bool received) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Formatting message to be printed...", mClassNamePrefix);
    // Remove spaces from the beggining and end
    std::string message = cmessage;
    const std::string delimiter = " ";
    std::string newMessageStr;
    for (auto charIterator = message.begin(); charIterator != message.end(); charIterator++) {
        if (*charIterator != delimiter.back()) {
            for (; charIterator != message.end(); charIterator++) {
                newMessageStr.push_back(*charIterator);
            }
            break;
        }
    }
    message = newMessageStr;
    if (message.back() == delimiter.back()) {
        message = message.substr(0, message.rfind(delimiter));
    }

    // Remove duplicated spaces
    newMessageStr.clear();
    for (auto charIterator = message.begin(); charIterator != message.end(); charIterator++) {
        if (*charIterator != delimiter.back()) {
            newMessageStr.push_back(*charIterator);
        } else {
            newMessageStr.push_back(delimiter.back());
            for (; charIterator != message.end() && *charIterator == delimiter.back(); charIterator++) {
            }
            if (charIterator != message.end()) {
                newMessageStr.push_back(*charIterator);
            } else {
                break;
            }
        }
    }
    message = newMessageStr;

    // Make sure message is not empty
    std::vector<std::string> lines;
    if (message.empty()) {
        lines.emplace_back(" ");
    } else {
        // Extract words from the message
        std::vector<std::string> words;
        size_t lastPosition = 0, currentPosition;
        while ((currentPosition = message.find(delimiter, lastPosition)) != std::string::npos) {
            std::string word = message.substr(lastPosition, currentPosition - lastPosition);
            lastPosition += (word.size() + 1);
            words.emplace_back(word);
        }
        words.emplace_back(message.substr(lastPosition, message.size()));

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
    auto time = std::chrono::system_clock::to_time_t(timestmap);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
    const auto timestamp = ss.str();

    // Print message based on side
    logger.info("{} Printing message...", mClassNamePrefix);
    if (received) {
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
