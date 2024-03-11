#pragma once
#include <functional>

using TTContactsCallbackQuit = std::function<bool()>;
using TTContactsCallbackDataProduced = std::function<void()>;
using TTContactsCallbackDataConsumed = std::function<void()>;
