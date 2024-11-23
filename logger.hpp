
#pragma once

#ifdef USE_BOOST
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#pragma message("Compiling with Boost support")
#else
#include <format>
#pragma message("Compiling without Boost support")
#endif

#include <fstream>
#include <filesystem> // C++17 for filesystem utilities
#include <string>
#include <mutex>
#include <iostream>
#include <stdexcept>

/**
 * @class Logger
 * @brief Singleton logger class providing thread-safe logging with various log levels.
 */
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
     * @brief Enable detailed logging with file and function information.
     * @param verbose Flag to enable verbosity.
     */
    void setVerbosity(bool verbose) {
        std::lock_guard<std::mutex> lock(mutex_);
        verbose_ = verbose;
    }

    /**
     * @brief Enable or disable console output for logs.
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
     * @param file The file name (optional, used if verbosity is enabled).
     * @param function The function name (optional, used if verbosity is enabled).
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

    std::string logFile_; ///< Path to the log file
    LogLevel minLogLevel_ = LogLevel::INFO; ///< Minimum log level
    bool logToConsole_ = false; ///< Flag for console logging
    size_t maxLogSize_ = 5 * 1024 * 1024; ///< Maximum log size (default: 5 MB)
    std::mutex mutex_; ///< Mutex for thread safety
    bool verbose_; ///< Flag to control verbosity
};

// Macro to simplify logging calls
#define SIELOG(level, message)     Logger::getInstance().log(Logger::LogLevel::level, message, __FILE__, __func__)

/**
 * @example
 * auto& logger = Logger::getInstance();
 * logger.setLogFile("log.txt");
 * logger.setLogLevel(Logger::LogLevel::DEBUG);
 * SIELOG(INFO, "This is an informational message.");
 */
