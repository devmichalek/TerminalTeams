#pragma once
#include <iostream>

enum class TTContactsStatus : int {
    STATE = 0,
    HEARTBEAT = 1,
    GOODBYE = 2
};

inline std::ostream& operator<<(std::ostream& os, const TTContactsStatus& rhs)
{
    switch (rhs) {
        case TTContactsStatus::STATE: os << "STATE"; break;
        case TTContactsStatus::HEARTBEAT: os << "HEARTBEAT"; break;
        case TTContactsStatus::GOODBYE: os << "GOODBYE"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}
