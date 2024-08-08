#pragma once
#include <string>
#include <string_view>
#include <chrono>
#include <cstring>
#include <cassert>
#include "TTChatMessageType.hpp"

using TTChatTimestamp = std::chrono::time_point<std::chrono::system_clock>;

class TTChatMessage {
public:
    TTChatMessage() = default;
    ~TTChatMessage() = default;
    TTChatMessage(const TTChatMessage&) = default;
    TTChatMessage(TTChatMessage&&) = default;
    TTChatMessage& operator=(const TTChatMessage&) = default;
    TTChatMessage& operator=(TTChatMessage&&) = default;
    void setType(TTChatMessageType type) { mType = type; }
    void setTimestamp(TTChatTimestamp timestamp) { mTimestamp = timestamp; }
    void setData(const std::string_view& data) {
        assert(data.size() <= MAX_DATA_LENGTH);
        mDataLength = data.size();
        memcpy(mData, data.data(), data.size());
    }
    TTChatMessageType getType() const { return mType; }
    TTChatTimestamp getTimestamp() const { return mTimestamp; }
    std::string getData() const { return std::string(mData, mDataLength); }
    static constexpr unsigned int MAX_DATA_LENGTH = 2048;
private:
    TTChatMessageType mType;
    TTChatTimestamp mTimestamp;
    unsigned int mDataLength;
    char mData[MAX_DATA_LENGTH];
};
