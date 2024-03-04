#pragma once
#include "TTContactsSettings.hpp"

class TTContacts {
public:
    TTContacts(TTContactsSettings settings);
    void run();
private:
    void* mSharedMessage;
};