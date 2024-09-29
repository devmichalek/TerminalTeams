#pragma once
#include <iostream>

enum class TTTextBoxStatus : unsigned int {
    UNDEFINED = 0,
    HEARTBEAT,
    CONTACTS_SELECT,
    MESSAGE,
    GOODBYE
};

inline std::ostream& operator<<(std::ostream& os, const TTTextBoxStatus& rhs)
{
    switch (rhs) {
        case TTTextBoxStatus::UNDEFINED: os << "UNDEFINED"; break;
        case TTTextBoxStatus::HEARTBEAT: os << "HEARTBEAT"; break;
        case TTTextBoxStatus::CONTACTS_SELECT: os << "CONTACTS_SELECT"; break;
        case TTTextBoxStatus::MESSAGE: os << "MESSAGE"; break;
        case TTTextBoxStatus::GOODBYE: os << "GOODBYE"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}
