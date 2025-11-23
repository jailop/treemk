# TreeMk - Markdown Editor

A wiki-Markdown text editor built with Qt, designed for organizing and managing interconnected notes.

![screenshot](screenshot.png)

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

Pandoc is required to expert notes to formats like HTML, PDF, and DOCX.
Pandoc needs to be installed separately. For example, using `chocolatey`
on Windows:

```powershell
choco install pandoc
```

### Qt Modules Required

- Qt6::Core
- Qt6::Gui
- Qt6::Widgets
- Qt6::WebEngineWidgets

## Building from Source

### Linux

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
```

### Windows

Requirements:
- Visual Studio 2019 or later (with C++ support)
- Qt 6 for Windows
- CMake

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt6_DIR if needed)
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"

# Build
cmake --build . --config Release

# Run
Release\treemk.exe
```

**Note for Windows**: After building, use `windeployqt` to deploy Qt dependencies:
```bash
windeployqt Release\treemk.exe
```

### macOS

Requirements:
- Xcode or Xcode Command Line Tools
- Qt 6 for macOS
- CMake

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

**Note for macOS**: After building, use `macdeployqt` to create a distributable app bundle:
```bash
macdeployqt treemk.app
```
