#pragma once
#include "TTContactsState.hpp"
#include "TTContactsStatus.hpp"
#include <string>

inline const unsigned int TTCONTACTS_DATA_MAX_LENGTH = 256;
inline const long TTCONTACTS_HEARTBEAT_TIMEOUT_MS = 500; // 0.5s

// Directional message
class TTContactsMessage final {
public:
    TTContactsMessage() = default;
    ~TTContactsMessage() = default;
    TTContactsMessage(const TTContactsMessage&) = default;
    TTContactsMessage(TTContactsMessage&&) = default;
    TTContactsMessage& operator=(const TTContactsMessage&) = default;
    TTContactsMessage& operator=(TTContactsMessage&&) = default;
    void setIdentity(size_t identity) { mIdentity = identity; }
    void setStatus(TTContactsStatus status) { mStatus = status; }
    void setState(TTContactsState state) { mState = state; }
    void setNickname(const std::string& nickname) {
        mDataLength = nickname.size();
        memset(&mData[0], 0, TTCONTACTS_DATA_MAX_LENGTH);
        memcpy(&mData[0], nickname.c_str(), mDataLength);
    }
    size_t getIdentity() const { return mIdentity; }
    TTContactsStatus getStatus() const { return mStatus; };
    TTContactsState getState() const { return mState; }
    std::string getNickname() const { return std::string(mData, mData + mDataLength); }
private:
    size_t mIdentity;
    TTContactsStatus mStatus;
    TTContactsState mState;
    unsigned int mDataLength;
    char mData[TTCONTACTS_DATA_MAX_LENGTH];
};
