/*
 * @file main.cpp
 * @brief Entry point demonstrating the Logger functionality with multithreading and crash handling.
 *
 * @version Updated: 2-Dec-2024
 * @author Mark Ocondi
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
 */


#include <iostream>
#include <thread>
#include <vector>
#include <csignal>
#include <cstdlib>
#include "logger.hpp" // Include the updated Logger header

#ifdef _WIN32
#include <windows.h>
#undef ERROR
#else
#include <unistd.h>
#endif


// Global flag to differentiate between normal and signal-based exits
bool exitingFromSignal = false;

/**
 * @brief Unified cleanup function for normal and signal-based exits.
 */
void cleanup() {
    auto& logger = Logger::getInstance();

    if (exitingFromSignal) {
        logger.log(Logger::LogLevel::CRITICAL, "Exiting due to a signal.");
    } else {
        logger.log(Logger::LogLevel::INFO, "Exiting normally.");
    }

    // Perform any other resource cleanup tasks here
    logger.stop();
    std::cout << "Cleanup completed." << std::endl;
}

/**
 * @brief Signal handler to handle termination signals.
 * @param signal The signal number.
 */
void signalHandler(int signal) {
    exitingFromSignal = true;

    // Log the signal type
    auto& logger = Logger::getInstance();
    switch (signal) {
        case SIGINT:
            logger.log(Logger::LogLevel::ERROR, "Received SIGINT (Interrupt)");
            break;
        case SIGTERM:
            logger.log(Logger::LogLevel::ERROR, "Received SIGTERM (Termination)");
            break;
#ifdef SIGSEGV
        case SIGSEGV:
            logger.log(Logger::LogLevel::CRITICAL, "Received SIGSEGV (Segmentation Fault)");
            break;
#endif
        default:
            logger.log(Logger::LogLevel::ERROR, "Received unknown signal");
            break;
    }

    // Call cleanup
    cleanup();

    // Exit gracefully
    exit(EXIT_FAILURE);
}

/**
 * @brief Registers signal handlers for supported signals.
 */
void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);  // Interrupt (Ctrl+C)
    std::signal(SIGTERM, signalHandler); // Termination request
#ifdef SIGSEGV
    std::signal(SIGSEGV, signalHandler); // Segmentation fault
#endif
#ifdef _WIN32
    // Windows-specific signals (optional)
    std::signal(SIGABRT, signalHandler); // Abort
#endif
}

void threadFunction(int threadId) {
    auto& logger = Logger::getInstance();

    SIELOG(INFO, ("Thread " + std::to_string(threadId) + " started.").c_str());
    SIELOG(DEBUG, ("Thread " + std::to_string(threadId) + " is running.").c_str());
    SIELOG(WARNING, ("Thread " + std::to_string(threadId) + " encountered a minor issue.").c_str());
    SIELOG(ERROR, ("Thread " + std::to_string(threadId) + " encountered an error.").c_str());
    SIELOG(INFO, ("Thread " + std::to_string(threadId) + " finished.").c_str());
}

int main() {
    auto& logger = Logger::getInstance();
    logger.setLogFile("app.log");
    logger.setLogLevel(Logger::LogLevel::DEBUG);
    logger.setLogToConsole(true);

    // Register cleanup for normal exit
    atexit(cleanup);

    // Setup signal handlers for abnormal termination
    setupSignalHandlers();

    const int numThreads = 5;
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

    SIELOG(INFO, "All threads finished. Check the log file: app.log");

    // Simulate a crash for testing (uncomment to test signal handling)
    // int* ptr = nullptr;
    // *ptr = 42;

    return 0;
}
