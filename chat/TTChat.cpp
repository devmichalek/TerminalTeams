#include "TTChat.hpp"
#include <chrono>
#include <thread>

TTChat::TTChat(size_t width, size_t height) {
	// ...
}

void TTChat::run() {
	// ...
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

void TTChat::clear() {
	// ...
	clearWindow();
}

void TTChat::clearWindow() const {
	system("clear");
}