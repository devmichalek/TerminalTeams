#pragma once
#include "TTContactsSettings.hpp"
#include "TTContactsMessage.hpp"
#include "TTContactsCallback.hpp"
#include <semaphore.h>
#include <vector>
#include <string>

class TTContacts {
public:
    explicit TTContacts(TTContactsSettings settings,
        TTContactsCallbackQuit callbackQuit,
        TTContactsCallbackOutStream& callbackOutStream = std::cout);
    // Receives main data and sends confirmation
    void run();
private:
    // Handles new message, return true if refresh is needed
    bool handle(const TTContactsMessage& message);
    // Refreshes window
    void refresh();
    // Callbacks
    TTContactsCallbackQuit mCallbackQuit;
    TTContactsCallbackOutStream& mCallbackOutStream;
    // IPC shared memory communication
    TTContactsMessage* mSharedMessage;
    sem_t* mDataProducedSemaphore;
    sem_t* mDataConsumedSemaphore;
    // Terminal Emulator window properties
    size_t mTerminalWidth;
    size_t mTerminalHeight;
    // Contacts data
    std::vector<std::tuple<size_t, std::string, TTContactsStatus>> mContacts;
};