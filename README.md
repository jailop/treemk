# TreeMk - Markdown Editor

A wiki-Markdown text editor built with Qt, designed for organizing and
managing interconnected notes.

![screenshot](screenshot.png)

## Features

- File system tree view for note organization
- Wiki-style linking between notes with `[[note]]` syntax
  - Click wiki-links in the preview to navigate to linked files
  - Ctrl+Click wiki-links in the editor to navigate to linked files
  - Ctrl+Enter keyboard shortcut to open wiki-link at cursor position
  - Automatically open files in new tabs or switch to existing tabs
- File inclusion with `[[!note]]` syntax - embeds content of linked file
  in preview
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
- pandoc (for export features)

For Qt, the following modules need to be available at compile time:

- Qt6::Core
- Qt6::Gui
- Qt6::Widgets
- Qt6::WebEngineWidgets

pandoc is required to export notes to formats like HTML, PDF, and DOCX.
pandoc needs to be installed separately.

To render the preview, the app uses external JavaScript libraries
accessed via CDN:

- KaTeX is used for rendering LaTeX formulas.
- highlight.js is used for syntax highlighting in code blocks.

## Building from Source

### Linux

For Debian/Ubuntu-based systems, install the required packages:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-webengine-dev pandoc
```

To build the app:

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run
./treemk

# To install system-wide (optional)
sudo cmake --install .
```

### Windows

Additional requirement requirement:

- Visual Studio 2019 or later (with C++ support)

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt6_DIR if needed)
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"

# Build
cmake --build .
```

### macOS

Additional requirement:

- Xcode or Xcode Command Line Tools

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust CMAKE_PREFIX_PATH if needed)
cmake .. -DCMAKE_PREFIX_PATH="~/Qt/6.x.x/macos"

# Build
cmake --build .

# Run
open treemk.app
```
