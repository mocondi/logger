/*
 * @file main.cpp
 * @brief Entry point demonstrating the Logger functionality with multithreading.
 *
 * @author Mark Ocondi
 * @date 20-Novwember-2024
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



#include <iostream>
#include <thread>
#include <vector>
#include <csignal>
#include <fcntl.h>  // For _open(), _O_* flags
#include <cstring>  // For strlen()

#ifdef _WIN32
#include <io.h>     // For _write() and _close() on Windows
#define O_WRONLY _O_WRONLY
#define O_CREAT  _O_CREAT
#define O_APPEND _O_APPEND
#define write _write
#define close _close
#else
#include <unistd.h> // For write() and close() on POSIX
#endif

#include "logger.hpp" // Include the logger header

int crashLogFd = -1;

/**
 * @class SignalSafeLogger
 * @brief Provides minimal, signal-safe logging functionality.
 *
 * @details
 * This logger is specifically designed for use in signal handlers. It avoids
 * unsafe operations such as dynamic memory allocation, locking, and standard
 * library I/O functions. Logs are written directly to `stderr` using `write`.
 */
class SignalSafeLogger {
public:
    /**
     * @brief Logs a message to `stderr`.
     * @param message The message to log.
     *
     * @details
     * This function is signal-safe and writes the message directly to `stderr`
     * without dynamic memory allocation or other non-signal-safe operations.
     */
    static void log(const char* message) {
        // Explicit cast to unsigned int for Windows compatibility
        write(fileno(stderr), message, static_cast<unsigned int>(strlen(message)));
        write(fileno(stderr), "\n", 1);
    }
    /**
     * @brief Logs an error message with an associated error code.
     * @param message The error message to log.
     * @param errorCode The associated error code (e.g., `errno`).
     *
     * @details
     * This function is signal-safe and logs the error message along with
     * the human-readable description of the error code.
     */
    static void logToFile(int fd, const char* message) {
        if (fd != -1) {
            write(fd, message, static_cast<unsigned int>(strlen(message)));
            write(fd, "\n", 1);
        }
    }
};


/**
 * @brief Sets up the crash log file by opening it and storing the file descriptor.
 * 
 * @details
 * Opens the file in append mode. If the file does not exist, it is created.
 * This function ensures the file is ready for signal-safe logging.
 */
void setupCrashLog() {
    crashLogFd = open("crash.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (crashLogFd == -1) {
        SignalSafeLogger::log("Failed to open crash log file.");
    }
}

/**
 * @brief Cleans up the crash log file by closing the file descriptor.
 * 
 * @details
 * Should be called during normal application exit to release resources.
 */
void cleanupCrashLog() {
    if (crashLogFd != -1) {
        close(crashLogFd);
    }
}

/**
 * @brief Signal handler to log the received signal and terminate the application.
 * @param signal The signal number (e.g., SIGSEGV, SIGABRT).
 *
 * @details
 * This function logs the type of signal received using `SignalSafeLogger`
 * and terminates the application safely using `_exit`.
 */
void signalHandler(int signal) {
    SignalSafeLogger::logToFile(crashLogFd, "Application crashed with signal:");
    switch (signal) {
        case SIGSEGV:
            SignalSafeLogger::logToFile(crashLogFd, "SIGSEGV (Segmentation Fault)");
            break;
        case SIGABRT:
            SignalSafeLogger::logToFile(crashLogFd, "SIGABRT (Abort)");
            break;
        case SIGFPE:
            SignalSafeLogger::logToFile(crashLogFd, "SIGFPE (Floating Point Exception)");
            break;
        case SIGILL:
            SignalSafeLogger::logToFile(crashLogFd, "SIGILL (Illegal Instruction)");
            break;
        case SIGINT:
            SignalSafeLogger::logToFile(crashLogFd, "SIGINT (Interrupt)");
            break;
        case SIGTERM:
            SignalSafeLogger::logToFile(crashLogFd, "SIGTERM (Termination)");
            break;
        default:
            SignalSafeLogger::logToFile(crashLogFd, "Unknown signal received");
            break;
    }

    _exit(EXIT_FAILURE);
}

/**
 * @brief Registers signal handlers for common crash-related signals.
 *
 * @details
 * This function sets up signal handlers for signals such as `SIGSEGV` (segmentation fault),
 * `SIGABRT` (abort), `SIGFPE` (floating-point exception), `SIGILL` (illegal instruction),
 * `SIGINT` (interrupt), and `SIGTERM` (termination).
 */
void setupSignalHandlers() {
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

// Simple crashing function
int crashApp() {
    // Simulate a segmentation fault
    int* ptr = nullptr;
    *ptr = 42;
    return 0;
}

/**
 * @brief Function to log messages from multiple threads.
 * @param threadId ID of the thread logging the messages.
 */
void threadFunction(int threadId) {
    auto& logger = Logger::getInstance();

    // Enable verbosity for this thread
    logger.setVerbosity(true);

    SIELOG(INFO, "Thread " + std::to_string(threadId) + " started.");
    SIELOG(DEBUG, "Thread " + std::to_string(threadId) + " is running.");
    SIELOG(WARNING, "Thread " + std::to_string(threadId) + " encountered a minor issue.");
    SIELOG(ERROR, "Thread " + std::to_string(threadId) + " encountered an error.");
    SIELOG(INFO, "Thread " + std::to_string(threadId) + " finished.");
}


/**
 * @brief Main function demonstrating the logger functionality with multithreading.
 * @return Exit code (0 for success).
 */
int main() {
    // Set up the logger
    auto& logger = Logger::getInstance();
    logger.setLogFile("logTest.log"); // Log file path
    logger.logToConsole(true);

    setupCrashLog();
    atexit(cleanupCrashLog);

    // Setup signal handlers
    setupSignalHandlers();

    // Number of threads for testing
    const int numThreads = 5;

    // Vector to hold threads
    std::vector<std::thread> threads;

    std::cout << "Starting threads..." << std::endl;

    // Create and start threads
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunction, i + 1);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    logger.setVerbosity(false);
    SIELOG(INFO, "This log will not include file or function info.");

    std::cout << "All threads finished. Check the log file: logTest.log" << std::endl;

    // Simulate a crash for testing
    crashApp();

    logger.log(Logger::LogLevel::INFO, "This message will not be logged.");

    return 0;
}
