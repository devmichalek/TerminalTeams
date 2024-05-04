#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace TTDiagnosticsHelper {
    inline std::string now()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");
        return ss.str();
    }

    inline std::string generateUniquePath(const std::string& filename, const std::string& extension) {
        return std::string("/tmp/").append(filename).append("-").append(now()).append(extension);
    }
}

