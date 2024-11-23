# Logger Project

A cross-platform generic logger for screen and file output, with optional Boost support.

## Features
- Thread-safe singleton design
- Log levels: INFO, WARNING, ERROR, DEBUG
- Optional verbosity for detailed logs (file, function names)
- Automatic log rotation
- Console and file logging
- Cross-platform compatibility

## Build Instructions

### Prerequisites
- C++17 compatible compiler
- (Optional) Boost libraries installed

### Building with CMake

#### With Boost (default):
```bash
cmake -S . -B build -DUSE_BOOST=ON
cmake --build build
```

#### Without Boost:
```bash
cmake -S . -B build -DUSE_BOOST=OFF
cmake --build build
```

### Building with Makefile

#### With Boost:
```bash
make USE_BOOST=1
```

#### Without Boost:
```bash
make USE_BOOST=0
```

#### Clean Build:
```bash
make clean
```

## Usage Example
Refer to `main.cpp` for a demonstration of the logger functionality, including:
- Creating logs from multiple threads
- Enabling verbosity
- Customizing log levels and file outputs

## Licensing
Refer to the `LICENSE` file for licensing details.
