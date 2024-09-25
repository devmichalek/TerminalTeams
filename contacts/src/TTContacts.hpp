#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsEntry.hpp"
#include "TTUtilsSharedMem.hpp"
#include "TTContactsMessage.hpp"
#include "TTUtilsOutputStream.hpp"
#include "TTDiagnosticsLogger.hpp"
#include "TTUtilsStopable.hpp"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <atomic>

class TTContacts :public TTUtilsStopable {
public:
    explicit TTContacts(const TTContactsSettings& settings, TTUtilsOutputStream& outputStream);
    virtual ~TTContacts();
    TTContacts(const TTContacts&) = delete;
    TTContacts(TTContacts&&) = delete;
    TTContacts& operator=(const TTContacts&) = delete;
    TTContacts& operator=(TTContacts&&) = delete;
    // Receives main data and sends confirmation
    virtual void run();
protected:
    TTContacts() = default;
private:
    // Handles new message, return true if refresh is needed
    bool handle(const TTContactsMessage& message);
    // Refreshes window
    bool refresh();
    // Output stream
    TTUtilsOutputStream& mOutputStream;
    // IPC shared memory communication
    std::shared_ptr<TTUtilsSharedMem> mSharedMem;
    // Thread concurrent message communication
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<TTContactsEntry> mEntries;
};