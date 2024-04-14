#pragma once
#include <functional>
#include <iostream>

using TTContactsCallbackQuit = std::function<bool()>;
using TTContactsCallbackDataProduced = std::function<void()>;
using TTContactsCallbackDataConsumed = std::function<void()>;
using TTContactsCallbackOutStream = std::ostream;
