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
#include <unistd.h> // For write
#include <cstring>  // For strerror

#include "logger.hpp" // Include the logger header

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
        write(STDERR_FILENO, message, strlen(message)); // Safe logging to stderr
        write(STDERR_FILENO, "\n", 1); // Add a newline
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
    static void logError(const char* message, int errorCode) {
        write(STDERR_FILENO, message, strlen(message));
        write(STDERR_FILENO, ": ", 2);
        const char* errorStr = strerror(errorCode); // Safe conversion of error code
        write(STDERR_FILENO, errorStr, strlen(errorStr));
        write(STDERR_FILENO, "\n", 1); // Add a newline
    }
};

/**
 * @brief Signal handler to log the received signal and terminate the application.
 * @param signal The signal number (e.g., SIGSEGV, SIGABRT).
 *
 * @details
 * This function logs the type of signal received using `SignalSafeLogger`
 * and terminates the application safely using `_exit`.
 */
void signalHandler([[maybe_unused]] int signal) {
    printf("signal called: %d\n", signal);
    // Log the signal using the signal-safe logger
    SignalSafeLogger::log("Application crashed with signal:");
    switch (signal) {
        case SIGSEGV:
            SignalSafeLogger::log("SIGSEGV (Segmentation Fault)");
            break;
        case SIGABRT:
            SignalSafeLogger::log("SIGABRT (Abort)");
            break;
        case SIGFPE:
            SignalSafeLogger::log("SIGFPE (Floating Point Exception)");
            break;
        case SIGILL:
            SignalSafeLogger::log("SIGILL (Illegal Instruction)");
            break;
        case SIGINT:
            SignalSafeLogger::log("SIGINT (Interrupt)");
            break;
        case SIGTERM:
            SignalSafeLogger::log("SIGTERM (Termination)");
            break;
        default:
            SignalSafeLogger::log("Unknown signal");
            break;
    }

    // Exit the application safely
    _exit(EXIT_FAILURE); // Exit without cleanup to avoid further issues
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

/**
 * @brief Main function demonstrating the logger functionality with multithreading.
 * @return Exit code (0 for success).
 */
int main() {
    // Set up the logger
    auto& logger = Logger::getInstance();
    logger.setLogFile("logTest.log"); // Log file path
    logger.logToConsole(true);

    // Setup signal handlers
    setupSignalHandlers();
/*
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
*/
    logger.setVerbosity(false);
    SIELOG(INFO, "This log will not include file or function info.");

    std::cout << "All threads finished. Check the log file: logTest.log" << std::endl;

    // Simulate a crash for testing
    int* ptr = nullptr;
    *ptr = 42; // This will trigger SIGSEGV

    logger.log(Logger::LogLevel::INFO, "This message will not be logged.");

    return 0;
}
