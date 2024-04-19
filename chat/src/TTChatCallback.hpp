#pragma once
#include <functional>

using TTChatCallbackQuit = std::function<bool()>;
using TTChatCallbackMessageSent = std::function<void()>;
using TTChatCallbackMessageReceived = std::function<void()>;