#pragma once
#include "TTContactsSettings.hpp"
#include "TTUtilsSharedMem.hpp"
#include "TTContactsMessage.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <memory>
#include <vector>
#include <string>
#include <functional>

using TTContactsCallbackQuit = std::function<bool()>;

class TTContacts {
public:
    explicit TTContacts(const TTContactsSettings& settings,
        TTContactsCallbackQuit callbackQuit,
        const TTUtilsOutputStream& outputStream);
    virtual ~TTContacts();
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
    const TTUtilsOutputStream& mOutputStream;
    // Logger
    inline static const std::string mClassNamePrefix = "TTContacts:";
    // IPC shared memory communication
    std::shared_ptr<TTUtilsSharedMem> mSharedMem;
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<std::tuple<size_t, std::string, TTContactsStatus>> mContacts;
};