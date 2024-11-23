
#ifdef _WIN32
#define BOOST_INCLUDE_PATH "C:/boost_1_86_0/boost/version.hpp"
#else
#define BOOST_INCLUDE_PATH "/mnt/c/boost_1_86_0/boost/version.hpp"
#endif

#include BOOST_INCLUDE_PATH

#include BOOST_INCLUDE_PATH "/boost/version.hpp"


#include <iostream>
#include <thread>
#include <vector>
#include "Logger.hpp" // Include the logger header

// Function to log messages from multiple threads
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

int main() {
    // Set up the logger
    auto& logger = Logger::getInstance();
    logger.setLogFile("logTest.log"); // Log file path; ensure it is writable
    logger.logToConsole(true);

    // Number of threads for testing
    const int numThreads = 5;

    // Vector to hold threads
    std::vector<std::thread> threads;

    std::cout << "Starting threads...\n";

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
    SIELOG(INFO, "This log will not include file or function info");

    std::cout << "All threads finished. Check the log file: test_log.txt\n";

    return 0;
}



