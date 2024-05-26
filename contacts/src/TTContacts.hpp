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

class TTContacts {
public:
    explicit TTContacts(const TTContactsSettings& settings, const TTUtilsOutputStream& outputStream);
    virtual ~TTContacts();
    TTContacts(const TTContacts&) = delete;
    TTContacts(const TTContacts&&) = delete;
    TTContacts operator=(const TTContacts&) = delete;
    TTContacts operator=(const TTContacts&&) = delete;
    // Receives main data and sends confirmation
    virtual void run();
    // Stops applications
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Returns number of contacts
    virtual size_t size() const;
private:
    // Handles new message, return true if refresh is needed
    bool handle(const TTContactsMessage& message);
    // Refreshes window
    void refresh();
    // Output stream
    const TTUtilsOutputStream& mOutputStream;
    // IPC shared memory communication
    std::shared_ptr<TTUtilsSharedMem> mSharedMem;
    // Thread concurrent message communication
    std::atomic<bool> mStopped;
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<std::tuple<size_t, std::string, TTContactsStatus>> mContacts;
};