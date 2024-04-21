#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsConsumer.hpp"
#include "TTContactsCallback.hpp"
#include "TTContactsOutputStream.hpp"
#include <memory>
#include <vector>
#include <string>

class TTContacts {
public:
    explicit TTContacts(const TTContactsSettings& settings,
        TTContactsCallbackQuit callbackQuit,
        const TTContactsOutputStream& outputStream);
    virtual ~TTContacts() {}
    TTContacts(const TTContacts&) = delete;
    TTContacts(const TTContacts&&) = delete;
    TTContacts operator=(const TTContacts&) = delete;
    TTContacts operator=(const TTContacts&&) = delete;
    // Receives main data and sends confirmation
    virtual void run();
    // Returns number of contacts
    virtual size_t size() const;
private:
    // Handles new message, return true if refresh is needed
    bool handle(const TTContactsMessage& message);
    // Refreshes window
    void refresh();
    // Callbacks
    TTContactsCallbackQuit mCallbackQuit;
    // Output stream
    const TTContactsOutputStream& mOutputStream;
    // IPC shared memory communication
    std::shared_ptr<TTContactsConsumer> mConsumer;
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<std::tuple<size_t, std::string, TTContactsStatus>> mContacts;
};