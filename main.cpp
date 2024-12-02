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
#include <fcntl.h> // For _open(), _O_* flags
#include <cstring> // For strlen()

#ifdef _WIN32
#include <io.h> // For _write() and _close() on Windows
#include <sys/stat.h>
#define O_WRONLY _O_WRONLY
#define O_CREAT  _O_CREAT
#define O_APPEND _O_APPEND
#define PERMISSIONS _S_IREAD | _S_IWRITE
#else
#include <unistd.h>
#define PERMISSIONS S_IRUSR | S_IWUSR
#endif

#include "logger.hpp" // Include the updated Logger header

int crashLogFd = -1;

// Signal-safe logger class
class SignalSafeLogger {
public:
    static void log(const char* message) {
        write(fileno(stderr), message, static_cast<unsigned int>(strlen(message)));
        write(fileno(stderr), "\n", 1);
    }

    static void logToFile(int fd, const char* message) {
        if (fd != -1) {
            write(fd, message, static_cast<unsigned int>(strlen(message)));
            write(fd, "\n", 1);
        }
    }
};

void setupCrashLog() {
#ifdef _WIN32
    crashLogFd = _open("crash.log", _O_WRONLY | _O_CREAT | _O_APPEND, PERMISSIONS);
#else
    crashLogFd = open("crash.log", O_WRONLY | O_CREAT | O_APPEND, PERMISSIONS);
#endif
    if (crashLogFd == -1) {
        SignalSafeLogger::log("Failed to open crash log file.");
    } else {
        SignalSafeLogger::log("Crash log file opened successfully.");
    }
}

void cleanupCrashLog() {
    if (crashLogFd != -1) {
#ifdef _WIN32
        _close(crashLogFd);
#else
        close(crashLogFd);
#endif
        SignalSafeLogger::log("Crash log file closed successfully.");
    }
}

void signalHandler(int signal) {
    SignalSafeLogger::logToFile(crashLogFd, "Application crashed with signal:");
    switch (signal) {
        case SIGSEGV:
            SignalSafeLogger::logToFile(crashLogFd, "SIGSEGV (Segmentation Fault)");
            break;
        case SIGABRT:
            SignalSafeLogger::logToFile(crashLogFd, "SIGABRT (Abort)");
            break;
        default:
            SignalSafeLogger::logToFile(crashLogFd, "Unknown signal received");
            break;
    }
    _exit(EXIT_FAILURE);
}

void setupSignalHandlers() {
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
}

void threadFunction(int threadId) {
    auto& logger = Logger::getInstance();

    SIELOG(INFO, "Thread " + std::to_string(threadId) + " started.");
    SIELOG(DEBUG, "Thread " + std::to_string(threadId) + " is running.");
    SIELOG(WARNING, "Thread " + std::to_string(threadId) + " encountered a minor issue.");
    SIELOG(ERROR, "Thread " + std::to_string(threadId) + " encountered an error.");
    SIELOG(INFO, "Thread " + std::to_string(threadId) + " finished.");
}

int main() {
    auto& logger = Logger::getInstance();
    logger.setLogFile("logTest.log");
    logger.setLogLevel(Logger::LogLevel::DEBUG);
    logger.setLogToConsole(true);

    setupCrashLog();
    atexit(cleanupCrashLog);
    setupSignalHandlers();

    const int numThreads = 5;
    std::vector<std::thread> threads;

    std::cout << "Starting threads..." << std::endl;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunction, i + 1);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    SIELOG(INFO, "All threads finished. Check the log file: logTest.log");

    // Simulate a crash for testing
    // Uncomment to test signal handling
    // int* ptr = nullptr;
    // *ptr = 42;

    return 0;
}
