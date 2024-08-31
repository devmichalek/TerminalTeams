#pragma once
#include <iostream>

class TTContactsState
{
public:
    enum Value : size_t
    {
        ACTIVE = 0, // Active contact, no unread/pending messages, not selected
        INACTIVE, // Inactive contact, no unread/pending messages, not selected
        SELECTED_ACTIVE, // Active contact, no unread/pending messages, selected
        SELECTED_INACTIVE, // Inactive contact, no unread/pending messages, selected
        UNREAD_MSG_ACTIVE, // Active contact, unread messages, no pending messages, not selected
        UNREAD_MSG_INACTIVE, // Inactive contact, unread messages, no pending messages, not selected
        //PENDING_MSG_ACTIVE, // Impossible
        PENDING_MSG_INACTIVE, // Inactive contact, no unread messages, pending messages, not selected
        //PENDING_MSG_UNREAD_MSG_ACTIVE // Impossible
        //PENDING_MSG_UNREAD_MSG_INACTIVE // Impossible
        //SELECTED_UNREAD_MSG_ACTIVE // Impossible
        //SELECTED_UNREAD_MSG_INACTIVE // Impossible
        //SELECTED_PENDING_MSG_ACTIVE // Impossible
        SELECTED_PENDING_MSG_INACTIVE // Inactive contact, no unread messages, pending messages, selected
        //SELECTED_PENDING_MSG_UNREAD_MSG_ACTIVE // Impossible
        //SELECTED_PENDING_MSG_UNREAD_MSG_INACTIVE // Impossible
    };
    TTContactsState() = default;
    constexpr TTContactsState(const Value& rhs) : mValue(rhs) {}
    ~TTContactsState() = default;
    TTContactsState(const TTContactsState&) = default;
    TTContactsState(TTContactsState&&) = default;
    TTContactsState& operator=(const TTContactsState&) = default;
    TTContactsState& operator=(TTContactsState&&) = default;
    constexpr operator Value() const { return mValue; }
    explicit operator bool() const = delete;
    constexpr bool operator==(const TTContactsState& rhs) const { return mValue == rhs.mValue; }
    constexpr bool operator!=(const TTContactsState& rhs) const { return mValue != rhs.mValue; }
    constexpr bool operator==(const Value& rhs) const { return mValue == rhs; }
    constexpr bool operator!=(const Value& rhs) const { return mValue != rhs; }
    constexpr bool isActive() const { return mValue == ACTIVE || mValue == SELECTED_ACTIVE || mValue == UNREAD_MSG_ACTIVE; }
    constexpr bool isInactive() const { return !isActive(); }
    constexpr bool isSelected() const { return mValue == SELECTED_ACTIVE || mValue == SELECTED_INACTIVE || mValue == SELECTED_PENDING_MSG_INACTIVE; }
private:
    Value mValue;
};

inline std::ostream& operator<<(std::ostream& os, const TTContactsState& rhs)
{
    switch (rhs) {
        case TTContactsState::ACTIVE: os << "ACTIVE"; break;
        case TTContactsState::INACTIVE: os << "INACTIVE"; break;
        case TTContactsState::SELECTED_ACTIVE: os << "SELECTED_ACTIVE"; break;
        case TTContactsState::SELECTED_INACTIVE: os << "SELECTED_INACTIVE"; break;
        case TTContactsState::UNREAD_MSG_ACTIVE: os << "UNREAD_MSG_ACTIVE"; break;
        case TTContactsState::UNREAD_MSG_INACTIVE: os << "UNREAD_MSG_INACTIVE"; break;
        case TTContactsState::PENDING_MSG_INACTIVE: os << "PENDING_MSG_INACTIVE"; break;
        case TTContactsState::SELECTED_PENDING_MSG_INACTIVE: os << "SELECTED_PENDING_MSG_INACTIVE"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}
