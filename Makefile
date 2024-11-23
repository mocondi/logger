# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Paths and options
SRC = main.cpp
TARGET = main
INCLUDES = -I. # Add the current directory for includes

# Optional Boost support
USE_BOOST ?= 1 # Set to 1 to enable Boost; 0 to disable
BOOST_CXXFLAGS =
BOOST_LDFLAGS =

ifeq ($(USE_BOOST), 1)
    BOOST_INCLUDE_PATH = C:/boost_1_81_0 # Update this path to your Boost include directory
    BOOST_LIB_PATH = C:/boost_1_81_0/stage/lib # Update this path if you built Boost libraries
    BOOST_CXXFLAGS = -DUSE_BOOST -I$(BOOST_INCLUDE_PATH)
    BOOST_LDFLAGS = -L$(BOOST_LIB_PATH) -lboost_system -lboost_date_time # Update libraries as needed
    $(info Compiling with Boost support)
else
    $(info Compiling without Boost support)
endif

# Final build flags
ALL_CXXFLAGS = $(CXXFLAGS) $(BOOST_CXXFLAGS) $(INCLUDES)
ALL_LDFLAGS = $(BOOST_LDFLAGS)

# Build target
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(ALL_CXXFLAGS) -o $@ $^ $(ALL_LDFLAGS)

clean:
	@if exist $(TARGET).exe del /f $(TARGET).exe
	@if exist $(TARGET) del /f $(TARGET)
