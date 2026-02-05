# LSP - Language Server Protocol Implementation

<!-- [![CI](https://github.com/FidelSch/LSP/actions/workflows/ci.yml/badge.svg)](https://github.com/FidelSch/LSP/actions/workflows/ci.yml) -->

A C++23 implementation of a Language Server Protocol server with modern C++ features including modules support.

## Features

- 🚀 Modern C++23 with modules (`-fmodules-ts`)
- 📦 Shared library (`libLSPP`) and standalone executable
- 🧪 Comprehensive test suite using GoogleTest
- 🔄 JSON message parsing with nlohmann/json
- 📝 Text document synchronization and management
- 🏗️ CMake-based build system with Ninja support

## Requirements

- **CMake:** 3.30 or higher
- **Compiler:**
  - GCC 11+ or Clang 15+ (C++23 with modules support)
  - Currently tested with GCC using `-std=gnu++23`
- **Build Tools:** Ninja (recommended) or Make
- **Dependencies:** Vendored as git submodules
  - [GoogleTest](https://github.com/google/googletest) 1.16.0
  - [nlohmann/json](https://github.com/nlohmann/json) 3.12.0

## Building

### Clone with Submodules

```bash
git clone --recursive https://github.com/FidelSch/LSPP
cd LSP
```

Or if already cloned:

```bash
git submodule update --init --recursive
```

### Configure and Build

```bash
# Configure with CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja

# Build all targets
cmake --build build

# The outputs will be in build/:
# - build/main           (LSP server executable)
# - build/libLSPP.so.*   (shared library)
# - build/test_*         (test executables)
```

## Testing

Run all tests with CTest:

```bash
cd build
ctest --output-on-failure --verbose
```

Or run individual test suites:

```bash
./build/test_message       # Message parsing tests
./build/test_server        # Server functionality tests
./build/test_json          # JSON serialization tests
./build/test_textDocument  # Text document handling tests
```

## Installation

```bash
cmake --install build --prefix /usr/local
```

This installs:

- Libraries → `/usr/local/lib/`
- Headers → `/usr/local/include/`
- CMake configs → `/usr/local/lib/cmake/LSPP/`
- pkg-config file → `/usr/local/share/pkgconfig/`

## Project Structure

```
LSP/
├── src/                    # Source files
│   ├── Message.cpp
│   ├── Server.cpp
│   ├── ProtocolStructures.cpp
│   ├── textDocument.cpp
│   └── main.cpp
├── include/                # Public headers
│   ├── Message.hpp
│   ├── Server.hpp
│   ├── ProtocolStructures.hpp
│   └── textDocument.hpp
├── test/                   # Test suite
│   ├── test_message.cpp
│   ├── test_server.cpp
│   ├── test_json.cpp
│   └── test_textDocument.cpp
├── deps/                   # Git submodules
│   ├── googletest/
│   └── json/
└── build/                  # Build output (generated)
```
