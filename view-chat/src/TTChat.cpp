#include "TTChat.hpp"
#include <ctime>
#include <iostream>
#include <chrono>
#include <mqueue.h>

TTChat::TTChat(TTChatSettings settings,
    TTChatCallbackQuit callbackQuit) :
		mCallbackQuit(callbackQuit),
		mWidth(settings.getTerminalWidth()),
		mHeight(settings.getTerminalWidth()),
		mSideWidth(mWidth * settings.getRatio()),
		mBlankLine(mWidth, ' ') {
	const std::string classNamePrefix = "TTChat: ";
	auto messageQueueName = settings.getMessageQueueName();
	auto messageQueueReversedName = messageQueueName + "-reversed";
	// Open message queue reversed
	errno = 0;
	for (auto attempt = TTCHAT_MESSAGE_QUEUE_READY_TRY_COUNT; attempt > 0; --attempt) {
		if (mCallbackQuit && mCallbackQuit()) {
			return; // Forced exit
		}
		struct mq_attr messageQueueReversedAttributes;
		messageQueueReversedAttributes.mq_maxmsg = TTCHAT_MESSAGE_MAX_NUM;
		messageQueueReversedAttributes.mq_msgsize = 0;
		messageQueueReversedAttributes.mq_flags = 0;
		mMessageQueueReversedDescriptor = mq_open(mMessageQueueReversedName.c_str(),
												O_RDWR | O_NONBLOCK,
												0644,
												&messageQueueReversedAttributes);
		if (mMessageQueueReversedDescriptor != -1) {
			break; // Success
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(TTCHAT_MESSAGE_QUEUE_READY_TIMEOUT_MS));
	}
	if (mMessageQueueReversedDescriptor == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to open reversed message queue, errno=" + std::to_string(errno));
	}

	// Open message queue
	struct mq_attr messageQueueAttributes;
	messageQueueAttributes.mq_maxmsg = TTCHAT_MESSAGE_MAX_NUM;
    messageQueueAttributes.mq_msgsize = TTCHAT_MESSAGE_MAX_LENGTH;
	messageQueueAttributes.mq_flags = 0;

	errno = 0;
	mMessageQueueDescriptor = mq_open(mMessageQueueName.c_str(),
                                      O_RDWR | O_NONBLOCK,
                                      0644,
                                      &messageQueueAttributes);
	if (mMessageQueueDescriptor == -1) {
		throw std::runtime_error(classNamePrefix + "Failed to open message queue, errno=" + std::to_string(errno));
	}
}

void TTChat::run() {
	if (mMessageQueueDescriptor == -1 || mMessageQueueReversedDescriptor == -1) {
		return;
	}

	while (true) {
		char messageBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
        for (auto i = TTCHAT_MESSAGE_RECEIVE_TRY_COUNT; i >= 0 && !mForcedQuit.load(); --i) {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                // Hard failure
				throw std::runtime_error({});
            }
            ts.tv_sec += TTCHAT_MESSAGE_RECEIVE_TIMEOUT_S;
            errno = 0;
            unsigned int priority = 0;
            auto result = mq_timedreceive(mMessageQueueDescriptor,
                                          messageBuffer,
                                          TTCHAT_MESSAGE_MAX_LENGTH,
                                          &priority,
                                          &ts);
            if (result == 0) {
				TTChatMessage message;
				std::memcpy(&message, messageBuffer, TTCHAT_MESSAGE_MAX_LENGTH);
				switch (message.type) {
					case TTChatMessageType::CLEAR:
						clear();
						break;
					case TTChatMessageType::SEND:
						print(message.data, message.timestamp, false);
						break;
					case TTChatMessageType::RECEIVE:
						print(message.data, message.timestamp, true);
						break;
					case TTChatMessageType::HEARTBEAT:
						[[fallthrough]]
					default:
						break;
				}

                i = TTCHAT_MESSAGE_RECEIVE_TRY_COUNT + 1;
                continue;
            }
            if (errno == EAGAIN || errno == ETIMEDOUT) {
                continue;
            }
            // Hard failure
            throw std::runtime_error({});
        }
	}
	mForcedQuit.store(true);
}

void TTChat::heartbeat() {
	try {
		char dummyBuffer[TTCHAT_MESSAGE_MAX_LENGTH];
		for (auto i = TTCHAT_MESSAGE_SEND_TRY_COUNT; i >= 0; --i) {
			if (mForcedQuit.load()) {
				throw std::runtime_error({});
			}
			struct timespec ts;
			if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
				throw std::runtime_error({}); // Hard failure
			}
			ts.tv_sec += TTCHAT_MESSAGE_SEND_TIMEOUT_S;
			errno = 0;
			result = mq_timedsend(mMessageQueueReversedDescriptor,
									&dummyBuffer[0],
									0,
									TTCHAT_MESSAGE_PRIORITY,
									&ts);
			if (result == 0) {
				i = TTCHAT_MESSAGE_SEND_TRY_COUNT + 1;
				continue;
			}
			if (errno == EAGAIN || errno == ETIMEDOUT) {
				continue; // Message queue is full or there is timeout, try again
			}
			// Hard failure
			throw std::runtime_error({});
		}
    } catch (...) {
        // ...
    }
    mForcedQuit.store(true);
}

void TTChat::print(const char* cmessage, TTChatTimestamp timestmap, bool received) {
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
	if (received) {
		std::cout << timestamp << "\n";
		for (auto &line : lines) {
			std::cout << line << "\n";
		}
	} else {
		std::cout << mBlankLine.substr(0, mBlankLine.size() - timestamp.size());
		std::cout << timestamp << "\n";
		for (auto &line : lines) {
			std::cout << mBlankLine.substr(0, mBlankLine.size() - line.size());
    		std::cout << line << "\n";
		}
	}
	std::cout << std::endl;
}

void TTChat::clear() {
	std::cout << "\033[2J\033[1;1H";
}


