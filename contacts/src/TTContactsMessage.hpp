#pragma once
#include "TTContactsState.hpp"
#include "TTContactsStatus.hpp"
#include <cstring>
#include <cassert>

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
    void setStatus(TTContactsStatus status) { mStatus = status; }
    void setState(TTContactsState state) { mState = state; }
    void setIdentity(size_t identity) { mIdentity = identity; }
    void setNickname(const std::string& nickname) {
        assert(nickname.size() < TTCONTACTS_DATA_MAX_LENGTH);
        mDataLength = nickname.size();
        memset(&mData[0], 0, TTCONTACTS_DATA_MAX_LENGTH);
        memcpy(&mData[0], nickname.c_str(), mDataLength);
    }
    TTContactsStatus getStatus() const { return mStatus; };
    TTContactsState getState() const { return mState; }
    size_t getIdentity() const { return mIdentity; }
    std::string getNickname() const { return std::string(mData, mData + mDataLength); }
private:
    TTContactsStatus mStatus;
    TTContactsState mState;
    size_t mIdentity;
    unsigned int mDataLength;
    char mData[TTCONTACTS_DATA_MAX_LENGTH];
};

inline std::ostream& operator<<(std::ostream& os, const TTContactsMessage& rhs)
{
    os << "{";
    switch (rhs.getStatus()) {
        case TTContactsStatus::STATE:
            os << "status: " << rhs.getStatus() << ", ";
            os << "state: " << rhs.getState() << ", ";
            os << "identity: " << rhs.getIdentity() << ", ";
            os << "nickname: " << rhs.getNickname();
            break;
        case TTContactsStatus::HEARTBEAT:
        case TTContactsStatus::GOODBYE:
            os << "status: " << rhs.getStatus();
            break;
        default:
            break;
    }
    os << "}";
    return os;
}

inline bool operator==(const TTContactsMessage& lhs, const TTContactsMessage& rhs) {
    if (lhs.getStatus() != rhs.getStatus()) {
        return false;
    }
    bool result = true;
    switch (lhs.getStatus()) {
        case TTContactsStatus::STATE:
            result &= (lhs.getState() == rhs.getState());
            result &= (lhs.getIdentity() == rhs.getIdentity());
            result &= (lhs.getNickname() == rhs.getNickname());
            break;
        case TTContactsStatus::HEARTBEAT:
            result = true;
            break;
        case TTContactsStatus::GOODBYE:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}
