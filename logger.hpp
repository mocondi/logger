/*
 * @file logger.hpp
 * @brief A generic, cross-platform logger for file and console output.
 *
 * @author Mark Ocondi
 * @date 20-November-2024
 *
 * @license
 * This file is part of the Logger Project.
 *
 * The Logger Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Logger Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Logger Project. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <cstdarg>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>


#define SIELOG(level, format, ...) \
    Logger::getInstance().log(Logger::LogLevel::level, format, ##__VA_ARGS__, __FILE__, __func__)

class Logger {
public:
    enum class LogLevel {
        TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename) {
        logFile_ = filename;
    }

    void setLogLevel(LogLevel level) {
        minLogLevel_ = level;
    }

    void setMaxLogSize(size_t bytes) {
        maxLogSize_ = bytes;
    }

    void setMaxBackups(int count) {
        maxBackups_ = count;
    }

    void setLogToConsole(bool enable) {
        logToConsole_ = enable;
    }

    void setFormat(const std::string& format) {
        logFormat_ = format;
    }

    void log(LogLevel level, const char* format, ...) {
        if (level < minLogLevel_) return;

        char buffer[1024]; // Adjust size as needed
        va_list args;
        va_start(args, format);
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        std::string formattedMessage = formatMessage(logFormat_, level, buffer, nullptr, nullptr);
        logAsync(formattedMessage);
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopLogging_ = true;
        }
        cv_.notify_all();
        if (loggingThread_.joinable()) {
            loggingThread_.join();
        }
    }

private:
    Logger()
        : loggingThread_(&Logger::loggingFunction, this) {}

    ~Logger() {
        stop();
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void loggingFunction() {
        while (!stopLogging_ || !logQueue_.empty()) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !logQueue_.empty() || stopLogging_; });

            while (!logQueue_.empty()) {
                std::string message = std::move(logQueue_.front());
                logQueue_.pop();
                lock.unlock();

                std::ofstream logStream(logFile_, std::ios_base::app);
                if (logStream.is_open()) {
                    logStream << message << std::endl;

                    if (logStream.tellp() >= static_cast<std::streampos>(maxLogSize_)) {
                        rotateLogs();
                    }
                }

                if (logToConsole_) {
                    std::cout << message << std::endl;
                }

                lock.lock();
            }
        }
    }

    void logAsync(const std::string& message) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            logQueue_.push(message);
        }
        cv_.notify_one();
    }

    std::string formatMessage(const std::string& format, LogLevel level, const std::string& message, const char* file, const char* function) {
        std::ostringstream result;
        std::string timestamp = getTimestamp();
        std::map<std::string, std::string> placeholders = {
            {"<TIMESTAMP>", timestamp},
            {"<LEVEL>", logLevelToString(level)},
            {"<MESSAGE>", message},
            {"<FILE>", file ? file : ""},
            {"<FUNCTION>", function ? function : ""}
        };

        std::string formatted = format;
        for (const auto& pair : placeholders) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;

            size_t pos;
            while ((pos = formatted.find(key)) != std::string::npos) {
                formatted.replace(pos, key.length(), value);
            }
        }

        return formatted;
    }

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        // Use a mutex to protect std::localtime (not thread-safe on some platforms)
        static std::mutex timeMutex;
        std::tm tm_now;
        {
            std::lock_guard<std::mutex> lock(timeMutex);
            tm_now = *std::localtime(&time_t_now);
        }

        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
        }
        return "UNKNOWN";
    }

    void rotateLogs() {
        for (int i = maxBackups_ - 1; i > 0; --i) {
            std::rename((logFile_ + "." + std::to_string(i)).c_str(),
                        (logFile_ + "." + std::to_string(i + 1)).c_str());
        }
        std::rename(logFile_.c_str(), (logFile_ + ".1").c_str());
    }

    std::queue<std::string> logQueue_;
    std::thread loggingThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopLogging_ = false;

    std::string logFile_ = "log.txt";
    LogLevel minLogLevel_ = LogLevel::INFO;
    bool logToConsole_ = true;
    size_t maxLogSize_ = 10 * 1024 * 1024;
    int maxBackups_ = 5;
    std::string logFormat_ = "[<TIMESTAMP>] [<LEVEL>] <MESSAGE>";
};


/**
 * @example
 * auto& logger = Logger::getInstance();
 * logger.setLogFile("log.txt");
 * logger.setLogLevel(Logger::LogLevel::DEBUG);
 * SIELOG(INFO, "This is an informational message.");
 */
