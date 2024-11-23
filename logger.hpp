/// @classification UNCLASSIFIED
/// @classification LOCKHEED MARTIN PROPRIETARY INFORMATION
///
/// @file
/// @copyright Lockheed Martin Corporation
/// @date 11/22/2024
///
/// @license
/// This computer software is PROPRIETARY INFORMATION of Lockheed Martin
/// Corporation and shall not be reproduced, disclosed, or used without
/// written permission of Lockheed Martin Corporation. All rights reserved.
///
/// @program TSS
///
/// @brief Logger is a generic logger class with enhanced features.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(__GNUC__)
    #include <boost/format.hpp>
    #include <boost/date_time.hpp>
    #pragma message "GCC compiler detected"
#elif defined(__clang__)
    #include <format>
    #pragma message "Clang compiler detected"
#elif defined(_MSC_VER)
    #include <boost/format.hpp>
    #include <boost/date_time.hpp>
    #pragma message "MSVC compiler detected"
#elif defined(__INTEL_COMPILER)
    #include <format>
    #pragma message "Intel C++ Compiler detected"
#endif

#include <fstream>
#include <filesystem> // C++17 for filesystem utilities
#include <string>
#include <mutex>
#include <iostream>
#include <stdexcept>

class Logger {
public:
    enum class LogLevel { INFO, WARNING, ERROR, DEBUG };

    /**
     * @brief Get the Singleton instance of the Logger.
     * @return Reference to the Logger instance.
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Set the log file path.
     * @param logFile Path to the log file.
     */
    void setLogFile(const std::string& logFile) {
        std::lock_guard<std::mutex> lock(mutex_);
        logFile_ = logFile;
    }

    /**
     * @brief Set the minimum log level to filter logs.
     * @param level Minimum log level to log.
     */
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        minLogLevel_ = level;
    }

    /**
     * @brief Set the logs to output calling file and function.
     * @param verbose flag to enable.
     */
    void setVerbosity(bool verbose) {
        std::lock_guard<std::mutex> lock(mutex_);
        verbose_ = verbose;
    }

    /**
     * @brief Enable or disable logging to the console.
     * @param enable True to enable console logging; false to disable.
     */
    void logToConsole(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        logToConsole_ = enable;
    }

    /**
     * @brief Log a message with a specified log level.
     * @param level The log level of the message.
     * @param message The message to log.
     */
    void log(LogLevel level, const std::string& message, 
            const char* file = nullptr, const char* function = nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Check if the log level is below the minimum log level
        if (level < minLogLevel_) {
            return;
        }

        // Rotate logs if necessary
        rotateLogs(maxLogSize_);

        // Prepare the log message
        std::ostringstream logMessage;

        // Get the current time
        time_t rawtime;
        time(&rawtime);

        struct tm timeInfo;
        char timeStr[20];
    #if defined(_WIN32) || defined(_WIN64)
        localtime_s(&timeInfo, &rawtime);
    #else
        localtime_r(&rawtime, &timeInfo);
    #endif
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);

        logMessage << timeStr;

        // Add log level
        switch (level) {
            case LogLevel::INFO:
                logMessage << " [INFO] ";
                break;
            case LogLevel::WARNING:
                logMessage << " [WARNING] ";
                break;
            case LogLevel::ERROR:
                logMessage << " [ERROR] ";
                break;
            case LogLevel::DEBUG:
                logMessage << " [DEBUG] ";
                break;
        }

        // Add file and function if verbosity is enabled
        if (verbose_ && file) {
            logMessage << "[" << file;
            if (function) {
                logMessage << "::" << function;
            }
            logMessage << "] ";
        }

        // Add the message
        logMessage << message;

        // Log to file
        std::ofstream logStream(logFile_, std::ios_base::app);
        if (logStream.is_open()) {
            logStream << logMessage.str() << std::endl;
        } else {
            std::cerr << "Error opening log file: " << logFile_ << std::endl;
        }

        // Log to console if enabled
        if (logToConsole_) {
            std::cout << logMessage.str() << std::endl;
        }
    }

    /**
     * @brief Set the maximum log size for rotation.
     * @param maxSize Maximum log file size in bytes.
     */
    void setMaxLogSize(size_t maxSize) {
        std::lock_guard<std::mutex> lock(mutex_);
        maxLogSize_ = maxSize;
    }

private:
    Logger() : verbose_(false) {} // Default to non-verbose logging
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Get the current timestamp as a string.
     * @return Formatted timestamp string.
     */
    std::string getTimestamp() {
        using namespace boost::posix_time;
        ptime now = second_clock::local_time();
        return to_simple_string(now);
    }

    /**
     * @brief Convert log level to a string.
     * @param level Log level to convert.
     * @return Corresponding string representation of the log level.
     */
    std::string toString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            case LogLevel::DEBUG:   return "DEBUG";
            default:                return "UNKNOWN";
        }
    }

    /**
     * @brief Rotate logs if the log file exceeds the maximum size.
     * @param maxSize Maximum allowed log file size in bytes.
     */
    void rotateLogs(size_t maxSize) {
        std::ifstream file(logFile_, std::ios::ate | std::ios::binary);
        if (file.is_open() && file.tellg() > static_cast<std::streampos>(maxSize)) {
            file.close();

            // Find the next available index for the backup file
            int index = 1;
            std::string backupFile;
            do {
                backupFile = logFile_ + "." + std::to_string(index);
                ++index;
            } while (std::filesystem::exists(backupFile));

            // Rename the current log file to the next available backup file
            std::rename(logFile_.c_str(), backupFile.c_str());
        }
    }

    std::string logFile_;       ///< Path to the log file
    LogLevel minLogLevel_ = LogLevel::INFO; ///< Minimum log level
    bool logToConsole_ = false; ///< Flag for console logging
    size_t maxLogSize_ = 5 * 1024 * 1024; ///< Maximum log size (default: 5 MB)
    std::mutex mutex_;          ///< Mutex for thread safety
    bool verbose_; // Flag to control verbosity
};

// Macro to simplify logging calls
#define SIELOG(level, message) \
    Logger::getInstance().log(Logger::LogLevel::level, message, __FILE__, __func__)

// Example Use:
/*
int main() {
    auto& logger = Logger::getInstance();
    logger.setLogFile("test_log.txt");

    // Enable verbosity
    logger.setVerbosity(true);

    SIELOG(INFO, "Main function started.");
    testFunction();
    SIELOG(ERROR, "An error occurred in main.");

    // Disable verbosity
    logger.setVerbosity(false);

    SIELOG(INFO, "This log will not include file or function info.");
    return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
/// UNCLASSIFIED
