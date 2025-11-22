# TreeMk - Markdown Editor

A wiki-Markdown text editor built with Qt, designed for organizing and managing interconnected notes.

## Features

- File system tree view for note organization
- Wiki-style linking between notes with `[[note]]` syntax
  - Click wiki-links in the preview to navigate to linked files
  - Ctrl+Click wiki-links in the editor to navigate to linked files
  - Ctrl+Enter keyboard shortcut to open wiki-link at cursor position
  - Automatically open files in new tabs or switch to existing tabs
- File inclusion with `[[!note]]` syntax - embeds content of linked file in preview
- Markdown preview with syntax highlighting
- Image embedding support
- LaTeX formula rendering (inline `$...$` and block `$$...$$`)
- Multi-tab editing
- Search and navigation tools
- Export to HTML, PDF, and DOCX (with Pandoc)

## Requirements

- Qt 6.2 or higher
- CMake 3.16 or higher
- C++17 compatible compiler
- Pandoc (for export features)

### Qt Modules Required

- Qt6::Core
- Qt6::Gui
- Qt6::Widgets
- Qt6::WebEngineWidgets

## Building from Source

### Linux/macOS

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run
./TreeMk
```
