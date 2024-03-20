#pragma once
#include "TTChatMessage.hpp"

using TTChatEntry = std::tuple<TTChatMessageType, std::string, TTChatTimestamp>
using TTChatEntries = std::vector<TTChatEntry>;
