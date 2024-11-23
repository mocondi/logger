# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(MyProject VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(main main.cpp)

# Include directories
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})
