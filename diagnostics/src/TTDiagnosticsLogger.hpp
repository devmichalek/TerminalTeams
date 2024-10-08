#pragma once

#ifdef TT_DIAGNOSTICS_LOGGER_DEBUG

#include "TTDiagnosticsHelper.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <string.h>

class TTDiagnosticsLogger {
public:
    ~TTDiagnosticsLogger() = default;

    template <typename... Args>
    void debug(spdlog::format_string_t<Args...> fmt, Args &&...args) const {
        mLogger->log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(spdlog::format_string_t<Args...> fmt, Args &&...args) const {
        mLogger->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(spdlog::format_string_t<Args...> fmt, Args &&...args) const {
        mLogger->log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(spdlog::format_string_t<Args...> fmt, Args &&...args) const {
        mLogger->log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(spdlog::format_string_t<Args...> fmt, Args &&...args) const {
        mLogger->log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }

    void setLevel(spdlog::level::level_enum logLevel) {
        mLogger->set_level(logLevel);
    }

    void setPattern(std::string pattern, spdlog::pattern_time_type timeType = spdlog::pattern_time_type::local) {
        mLogger->set_pattern(pattern, timeType);
    }

    static const TTDiagnosticsLogger& getInstance() {
        return mInstance;
    }

private:
    explicit TTDiagnosticsLogger(std::string name, size_t maxSize = 1048576 * 5) {
        const auto uniquePath = TTDiagnosticsHelper::generateUniquePath(name, ".log.txt");
        mLogger = spdlog::rotating_logger_mt(name, uniquePath.c_str(), maxSize, 1);
    }
    TTDiagnosticsLogger(const TTDiagnosticsLogger&) = delete;
    TTDiagnosticsLogger(TTDiagnosticsLogger&&) = delete;
    TTDiagnosticsLogger& operator=(const TTDiagnosticsLogger&) = delete;
    TTDiagnosticsLogger& operator=(TTDiagnosticsLogger&&) = delete;

    std::shared_ptr<spdlog::logger> mLogger;
    static TTDiagnosticsLogger mInstance;
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define _LOG_GEN_(FUN_NAME, FORMAT_STR) FUN_NAME("[{}:{}] " FORMAT_STR, __FILENAME__, __LINE__)
#define _LOG_GEN_MANY(FUN_NAME, FORMAT_STR, ...) FUN_NAME("[{}:{}] " FORMAT_STR, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(FORMAT_STR, ...) _LOG_GEN_ ## __VA_OPT__(MANY) (TTDiagnosticsLogger::getInstance().debug, FORMAT_STR __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO(FORMAT_STR, ...) _LOG_GEN_ ## __VA_OPT__(MANY) (TTDiagnosticsLogger::getInstance().info, FORMAT_STR __VA_OPT__(,) __VA_ARGS__)
#define LOG_WARNING(FORMAT_STR, ...) _LOG_GEN_ ## __VA_OPT__(MANY) (TTDiagnosticsLogger::getInstance().warning, FORMAT_STR __VA_OPT__(,) __VA_ARGS__)
#define LOG_ERROR(FORMAT_STR, ...) _LOG_GEN_ ## __VA_OPT__(MANY) (TTDiagnosticsLogger::getInstance().error, FORMAT_STR __VA_OPT__(,) __VA_ARGS__)
#define LOG_DECLARE(FILENAME) TTDiagnosticsLogger TTDiagnosticsLogger::mInstance(FILENAME)

#else

#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARNING(...)
#define LOG_ERROR(...)
#define LOG_DECLARE(FILENAME)

#endif
