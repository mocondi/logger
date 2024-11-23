

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

void testWithThreads(const int & numThreads)
{
    // Vector to hold threads
    std::vector<std::thread> threads;

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
}

void basicTest()
{

}

int main() {
    // Set up the logger
    auto& logger = Logger::getInstance();
    // Log file path; ensure it is writable
    logger.setLogFile("logTest.log"); 
    // Set to output to console
    logger.logToConsole(true);

    // Basic test
    std::cout << "Starting basic test...\n";
    basicTest();

    // Thread test
    std::cout << "Starting thread test...\n";
    testWithThreads(5);

    logger.setVerbosity(false);
    SIELOG(INFO, "This log will not include file or function info");

    std::cout << "Finished. Check the log file: logTest.log\n";

    return 0;
}



