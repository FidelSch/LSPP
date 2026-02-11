# LSPP Examples

This directory contains example implementations demonstrating how to use the LSPP library.

## simple_hover_server.cpp

A minimal LSP server that implements hover functionality. This example shows:

- How to create a custom server by inheriting from `LSPServer`
- How to implement the `hoverCallback` method to provide custom responses
- How to access document content and get the word under the cursor
- How to initialize and run the server

### Building

From the project root:

```bash
mkdir build && cd build
cmake ..
make simple_hover_server
```

### Running

```bash
./build/simple_hover_server
```

The server communicates via stdin/stdout using the Language Server Protocol.

### Usage example: Neovim

Configure Neovim to use this server:

```lua
vim.lsp.config['LSPP'] = {
  cmd = { '/path/to/build/simple_hover_server' },
  filetypes = { 'some_filetype' },
  root_markers = { { 'env', 'requirements.txt' }, '.git' },
  settings = {},
}
vim.lsp.enable 'LSPP'
```

Open a file and hover over text to see the response from the server (default K in neovim).
