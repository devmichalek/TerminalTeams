#pragma once

#ifdef TT_DIAGNOSTICS_LOGGER_DEBUG

#include <string>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

class TTDiagnosticsLogger {
 public:
    explicit TTDiagnosticsLogger(std::string name, size_t maxSize = 1048576 * 5) {
        const auto uniquePath = generateUniquePath(name);
        mLogger = spdlog::rotating_logger_mt(name, uniquePath.c_str(), maxSize, 1);
    }

    ~TTDiagnosticsLogger() = default;

    template <typename... Args>
    void debug(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        mLogger->log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        mLogger->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        mLogger->log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        mLogger->log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        mLogger->log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }

    void setLevel(spdlog::level::level_enum logLevel) {
        mLogger->set_level(logLevel);
    }

    void setPattern(std::string pattern, spdlog::pattern_time_type timeType = spdlog::pattern_time_type::local) {
        mLogger->set_pattern(pattern, timeType);
    }

 private:
    TTDiagnosticsLogger(const TTDiagnosticsLogger&) = delete;
    TTDiagnosticsLogger(const TTDiagnosticsLogger&&) = delete;
    TTDiagnosticsLogger operator=(const TTDiagnosticsLogger&) = delete;
    TTDiagnosticsLogger operator=(const TTDiagnosticsLogger&&) = delete;

    std::string generateUniquePath(const std::string& name) {
        return std::string("/tmp/").append(name).append("-").append(now()).append(".log.txt");
    }

    std::string now()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");
        return ss.str();
    }
    std::shared_ptr<spdlog::logger> mLogger;
};

#else

#include <string>
#include <spdlog/spdlog.h>

class TTDiagnosticsLogger {
 public:
    explicit TTDiagnosticsLogger(std::string name, size_t maxSize = 1048576 * 5) {
    }
    ~TTDiagnosticsLogger() = default;

    template <typename... Args>
    void debug(Args &&...args) {
    }

    template <typename... Args>
    void info(Args &&...args) {
    }

    template <typename... Args>
    void warning(Args &&...args) {
    }

    template <typename... Args>
    void error(Args &&...args) {
    }

    template <typename... Args>
    void critical(Args &&...args) {
    }

    void setLevel(spdlog::level::level_enum logLevel) {
    }

    void setPattern(std::string pattern, spdlog::pattern_time_type timeType = spdlog::pattern_time_type::local) {
    }

 private:
    TTDiagnosticsLogger(const TTDiagnosticsLogger&) = delete;
    TTDiagnosticsLogger(const TTDiagnosticsLogger&&) = delete;
    TTDiagnosticsLogger operator=(const TTDiagnosticsLogger&) = delete;
    TTDiagnosticsLogger operator=(const TTDiagnosticsLogger&&) = delete;
};

#endif