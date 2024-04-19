#pragma once
#include "TTChatMessage.hpp"
#include <string>
#include <vector>

using TTChatEntry = std::tuple<TTChatMessageType, std::string, TTChatTimestamp>;
using TTChatEntries = std::vector<TTChatEntry>;
