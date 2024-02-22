#include "TTChat.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

void TTChat::print(const TTChatMessage& message) {
	// Extract string message
	auto messageStr = message.message;

	// Remove spaces from the beggining and end
	const std::string delimiter = " ";
	std::string newMessageStr;
	for (auto charIterator = messageStr.begin(); charIterator != messageStr.end(); charIterator++) {
		if (*charIterator != delimiter.back()) {
			for (; charIterator != messageStr.end(); charIterator++) {
				newMessageStr.push_back(*charIterator);
			}
			break;
		}
	}
	messageStr = newMessageStr;
	if (messageStr.back() == delimiter.back()) {
		messageStr = messageStr.substr(0, messageStr.rfind(delimiter));
	}

	// Remove duplicated spaces
	newMessageStr.clear();
	for (auto charIterator = messageStr.begin(); charIterator != messageStr.end(); charIterator++) {
		if (*charIterator != delimiter.back()) {
			newMessageStr.push_back(*charIterator);
		} else {
			newMessageStr.push_back(delimiter.back());
			for (; charIterator != messageStr.end() && *charIterator == delimiter.back(); charIterator++) {
			}
			if (charIterator != messageStr.end()) {
				newMessageStr.push_back(*charIterator);
			} else {
				break;
			}
		}
	}
	messageStr = newMessageStr;

	// Make sure message is not empty
	std::vector<std::string> lines;
	if (messageStr.empty()) {
		lines.emplace_back(" ");
	} else {
		// Extract words from the message
		std::vector<std::string> words;
		size_t lastPosition = 0, currentPosition;
		while ((currentPosition = messageStr.find(delimiter, lastPosition)) != std::string::npos) {
			std::string word = messageStr.substr(lastPosition, currentPosition - lastPosition);
			lastPosition += (word.size() + 1);
			words.emplace_back(word);
		}
		words.emplace_back(messageStr.substr(lastPosition, messageStr.size()));

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
	auto time = std::chrono::system_clock::to_time_t( message.timestamp);
	std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
	const auto timestamp = ss.str();

	// Print message based on side
	if (message.side == TTChatSide::LEFT) {
		std::cout << timestamp << std::endl;
		for (auto &line : lines) {
			std::cout << line << std::endl;
		}
	} else if (message.side == TTChatSide::RIGHT) {
		std::cout << mBlankLine.c_str() + (mBlankLine.size() - (mWidth - timestamp.size()));
		std::cout << timestamp << std::endl;
		for (auto &line : lines) {
			std::cout << mBlankLine.c_str() + (mBlankLine.size() - (mWidth - line.size()));
			std::cout << line << std::endl;
		}
	}
	std::cout << std::endl;
}

void TTChat::print(const TTChatMessages& messages) {
	for (auto &message : messages) {
		print(message);
	}
}

void TTChat::clear() {
	system("clear");
}
