# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(MyProject VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Option to enable or disable Boost
option(USE_BOOST "Enable Boost Library" ON)

# Find Boost if enabled
if(USE_BOOST)
    find_package(Boost REQUIRED) # You can add OPTIONAL_COMPONENTS for specific libraries
    if(Boost_FOUND)
        message(STATUS "Boost found at: ${Boost_INCLUDE_DIRS}")
        include_directories(${Boost_INCLUDE_DIRS})
        add_definitions(-DUSE_BOOST) # Define a macro for conditional compilation
    else()
        message(FATAL_ERROR "Boost requested but not found!")
    endif()
else()
    message(STATUS "Building without Boost support.")
endif()

add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# Add executable
add_executable(loggerTest main.cpp)

# Include directories for the project
target_include_directories(loggerTest PRIVATE ${CMAKE_SOURCE_DIR})

# Link Boost libraries if enabled
if(USE_BOOST)
    target_link_libraries(loggerTest Boost::boost) # Update this if specific Boost libraries are needed
endif()
