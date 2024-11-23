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
 * @brief Main function demonstrating the logger functionality with multithreading.
 * @return Exit code (0 for success).
 */
int main() {
    // Set up the logger
    auto& logger = Logger::getInstance();
    logger.setLogFile("logTest.log"); // Log file path
    logger.logToConsole(true);

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

    return 0;
}
