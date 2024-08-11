#pragma once
#include <sstream>
#include <chrono>
#include <iostream>
#include <iomanip>

using TTChatTimestampRaw = std::chrono::time_point<std::chrono::system_clock>;

class TTChatTimestamp {
public:
    TTChatTimestamp() = default;
    TTChatTimestamp(const TTChatTimestamp&) = default;
    TTChatTimestamp(TTChatTimestamp&&) = default;
    TTChatTimestamp(const TTChatTimestampRaw& rhs) : mData(rhs) {}
    TTChatTimestamp(TTChatTimestampRaw&& rhs) : mData(rhs) {}
    ~TTChatTimestamp() = default;
    TTChatTimestamp& operator=(const TTChatTimestamp&) = default;
    TTChatTimestamp& operator=(TTChatTimestamp&&) = default;
    TTChatTimestamp& operator=(const TTChatTimestampRaw& rhs) {
        mData = rhs;
        return *this;
    }
    TTChatTimestamp& operator=(TTChatTimestampRaw&& rhs) {
        mData = rhs;
        return *this;
    }
    operator std::string() const {
        const auto time = std::chrono::system_clock::to_time_t(mData);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
        return ss.str();
    }
    bool operator==(const TTChatTimestamp& rhs) {return mData == rhs.mData;}
    bool operator!=(const TTChatTimestamp& rhs) {return !(mData == rhs.mData);}
private:
    TTChatTimestampRaw mData;
};

inline std::ostream& operator<<(std::ostream& os, const TTChatTimestamp& rhs)
{
    os << static_cast<std::string>(rhs);
    return os;
}
