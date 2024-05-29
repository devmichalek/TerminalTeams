#pragma once
#include <cstring>

enum class TTContactsStatus : size_t {
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
    SELECTED_PENDING_MSG_INACTIVE, // Inactive contact, no unread messages, pending messages, selected
    //SELECTED_PENDING_MSG_UNREAD_MSG_ACTIVE // Impossible
    //SELECTED_PENDING_MSG_UNREAD_MSG_INACTIVE // Impossible
    HEARTBEAT
};
