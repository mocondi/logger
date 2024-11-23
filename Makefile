# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
BOOST_INCLUDE = -I"C:/boost_1_86_0"      # Boost include path
BOOST_LIB = -L"C:/boost_1_86_0/stage/lib" # Boost library path

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    RM = del /Q
    TARGET = test.exe
else
    RM = rm -f
    TARGET = test
    BOOST_INCLUDE = -I/mnt/c/boost_1_86_0
    BOOST_LIB = -L/mnt/c/boost_1_86_0/stage/lib
endif

# Source and header files
SRCS = main.cpp
HEADERS = logger.hpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE) -o $(TARGET) $(OBJS) $(BOOST_LIB)

# Compile source files into object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE) -c $< -o $@

# Clean build files
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean
