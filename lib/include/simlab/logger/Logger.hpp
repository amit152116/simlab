#pragma once

#define FMT_HEADER_ONLY

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

namespace Logger {

    enum class LogLevel : uint8_t { DEBUG, INFO, WARN, ERROR, FATAL };

    class Logger {
      public:

        explicit Logger(std::ofstream logFile) : logFile_(std::move(logFile)) {}

        Logger(Logger&&)                    = delete;
        auto operator=(Logger&&) -> Logger& = delete;

        Logger(const Logger&)                    = delete;
        auto operator=(const Logger&) -> Logger& = delete;

        static auto instance() -> Logger& {
            static Logger logger;
            return logger;
        }

        template <typename... Args>
        void debug(fmt::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void info(fmt::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warn(fmt::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::WARN, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void fatal(fmt::format_string<Args...> fmt, Args&&... args) {
            log(LogLevel::FATAL, fmt, std::forward<Args>(args)...);
        }

        void setLogFile(const std::string& filename) {
            if (logFile_.is_open()) {
                logFile_.close();
            }
            logFile_.open(filename, std::ios::app);
        }

        void setLevel(LogLevel level) {
            currentLevel_ = level;
        }

      private:

        Logger() = default;

        ~Logger() {
            if (logFile_.is_open()) {
                logFile_.close();
            }
        }

        template <typename... Args>
        void log(LogLevel level, fmt::format_string<Args...> fmt,
                 Args&&... args) {
            if (level < currentLevel_) {
                return;
            }

            auto now       = std::chrono::system_clock::now();
            auto timestamp = std::chrono::system_clock::to_time_t(now);

            std::string levelStr   = getLevelString(level);
            fmt::color  levelColor = getLevelColor(level);

            // Format the message
            std::string message = fmt::format(fmt, std::forward<Args>(args)...);

            // Format the final log line
            std::string logLine =
                fmt::format("[{}] [{}] {}", levelStr, timestamp, message);

            // Console output with colors
            fmt::print(fg(levelColor), "{}\n", logLine);

            // File output without colors
            if (logFile_.is_open()) {
                logFile_ << logLine;
                logFile_.flush();
            }
        }

        static auto getLevelString(LogLevel level) -> std::string {
            switch (level) {
                case LogLevel::DEBUG:
                    return "DEBUG";
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::WARN:
                    return "WARN";
                case LogLevel::ERROR:
                    return "ERROR";
                case LogLevel::FATAL:
                    return "FATAL";
                default:
                    return "UNKNOWN";
            }
        }

        static auto getLevelColor(LogLevel level) -> fmt::color {
            switch (level) {
                case LogLevel::DEBUG:
                    return fmt::color::dark_gray;
                case LogLevel::INFO:
                    return fmt::color::light_green;
                case LogLevel::WARN:
                    return fmt::color::yellow;
                case LogLevel::ERROR:
                    return fmt::color::red;
                case LogLevel::FATAL:
                    return fmt::color::purple;
                default:
                    return fmt::color::white;
            }
        }

        LogLevel      currentLevel_ = LogLevel::DEBUG;
        std::ofstream logFile_;
    };

    // Global logger access function
    inline auto getLogger() -> Logger& {
        return Logger::instance();
    }

}  // namespace Logger
