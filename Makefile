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
    BOOST_CXXFLAGS = -DUSE_BOOST -I/usr/include # Update this path for your Boost installation
    BOOST_LDFLAGS = -L/usr/lib -lboost_system -lboost_date_time # Update these libraries as needed
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
	rm -f $(TARGET)
