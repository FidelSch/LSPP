# LSPP - Language Server Protocol Library

<!-- [![CI](https://github.com/FidelSch/LSP/actions/workflows/ci.yml/badge.svg)](https://github.com/FidelSch/LSP/actions/workflows/ci.yml) -->

A C++23 library for building Language Server Protocol (LSP) servers. LSPP provides a clean API for implementing custom language servers with minimal boilerplate.

## Features

- 🚀 Modern C++23 API with type safety
- 📦 Shared library (`libLSPP.so`) for easy integration
- 🎯 Simple callback-based architecture - override what you need
- 🔄 Automatic JSON-RPC message handling
- 📝 Built-in text document synchronization
- 🧪 Comprehensive test suite using GoogleTest
- 🏗️ CMake-based build system with installation support
- 📚 Example implementations included

## Requirements

- **CMake:** 3.30 or higher
- **Compiler:**
  - GCC 11+ or Clang 15+ (C++23 support)
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
# - build/libLSPP.so.*            (shared library)
# - build/simple_hover_server     (example LSP server)
# - build/test_*                  (test executables)
```

## Quick Start

Here's a minimal LSP server that provides hover functionality:

```cpp
#include "Server.hpp"

class MyServer : public LSPServer {
public:
    MyServer() {
        // Register hover callback using Method enum
        registerCallback<hoverParams, std::optional<hoverResult>>(
            Message::Method::HOVER,
            [this](const hoverParams &params) -> std::optional<hoverResult> {
                auto docOpt = m_documentHandler.getOpenDocument(params.textDocument.uri);
                if (!docOpt) return std::nullopt;

                std::string word = docOpt->get().wordUnderCursor(
                    params.position.line,
                    params.position.character
                );

                return hoverResult{
                    {MarkupKind::PlainText, "Hover info for: " + word},
                    std::nullopt
                };
            });
    }
};

int main() {
    MyServer server;
    server.init(ServerCapabilities::hoverProvider);
    return server.exit();
}
```

Build and link against `libLSPP`:

```bash
g++ -std=c++23 my_server.cpp -o my_server -lLSPP
```

See [examples/](examples/) for more complete examples.

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
sudo cmake --install build --prefix /usr/local
```

This installs:

- Shared library → `/usr/local/lib/libLSPP.so`
- Headers → `/usr/local/include/`
- CMake config → `/usr/local/lib/cmake/LSPP/`

### Using LSPP in Your Project

With CMake:

```cmake
find_package(LSPP REQUIRED)
target_link_libraries(your_target PRIVATE LSPP::LSPP)
```

Or compile manually:

```bash
g++ -std=c++23 my_server.cpp -o my_server -lLSPP
```

## Project Structure

```
LSP/
├── include/                # Public API headers
│   ├── Server.hpp              # Main LSPServer class
│   ├── Message.hpp             # LSP message handling
│   ├── ProtocolStructures.hpp  # LSP types and structures
│   └── textDocument.hpp        # Document management
├── src/                    # Library implementation
│   ├── Server.cpp
│   ├── Message.cpp
│   ├── ProtocolStructures.cpp
│   └── textDocument.cpp
├── examples/               # Example implementations
│   ├── simple_hover_server.cpp # Basic hover provider
│   └── README.md               # Examples documentation
├── test/                   # Test suite
│   ├── test_server.cpp
│   ├── test_message.cpp
│   ├── test_json.cpp
│   └── test_textDocument.cpp
├── deps/                   # Git submodules
│   ├── googletest/
│   └── json/
└── CMakeLists.txt          # Build configuration
```

## API Overview

LSPP uses a **template-based callback registration system**. Register callbacks for LSP methods in your constructor:

### Registering Callbacks

```cpp
class MyLSPServer : public LSPServer {
public:
    MyLSPServer() {
        // Option 1: Register using Method enum (recommended)
        registerCallback<hoverParams, std::optional<hoverResult>>(
            Message::Method::HOVER,
            [this](const hoverParams& p) { return handleHover(p); }
        );

        registerCallback<definitionParams, definitionResult>(
            Message::Method::DEFINITION,
            [this](const definitionParams& p) { return handleDefinition(p); }
        );

        // Option 2: Register using method string
        registerCallback<hoverParams, std::optional<hoverResult>>(
            "textDocument/hover",
            [this](const hoverParams& p) { return handleHover(p); }
        );
    }
};
```

### Available LSP Methods

Common methods (use `Message::Method::` enum):

- `HOVER` - Provide hover information
- `DEFINITION` - Go to definition
- `DECLARATION` - Go to declaration
- `COMPLETION` - Code completion
- `SIGNATURE_HELP` - Signature help
- `REFERENCES` - Find references
- And 40+ more LSP methods...
  std::string content = doc.getContent();
  }

````

## Using with Editors

### Neovim

```lua
vim.lsp.config['LSPP'] = {
  cmd = { '/path/to/your/lsp/server' },
  filetypes = { 'your_language' },
  root_markers = { '.git' },
}
vim.lsp.enable 'LSPP'
````

### VS Code

Add to your extension's `package.json`:

```json
{
  "contributes": {
    "languages": [
      {
        "id": "your_language",
        "extensions": [".ext"]
      }
    ],
    "configuration": {
      "title": "Your LSP",
      "properties": {
        "yourLSP.serverPath": {
          "type": "string",
          "default": "/path/to/server"
        }
      }
    }
  }
}
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
