#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsConsumer.hpp"
#include "TTContactsCallback.hpp"
#include <memory>
#include <vector>
#include <string>

class TTContacts {
public:
    explicit TTContacts(TTContactsSettings settings,
        TTContactsCallbackQuit callbackQuit,
        TTContactsCallbackOutStream& callbackOutStream = std::cout);
    // Receives main data and sends confirmation
    void run();
    // Returns number of contacts
    size_t size() const;
private:
    // Handles new message, return true if refresh is needed
    bool handle(const TTContactsMessage& message);
    // Refreshes window
    void refresh();
    // Callbacks
    TTContactsCallbackQuit mCallbackQuit;
    TTContactsCallbackOutStream& mCallbackOutStream;
    // IPC shared memory communication
    std::unique_ptr<TTContactsConsumer> mConsumer;
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<std::tuple<size_t, std::string, TTContactsStatus>> mContacts;
};