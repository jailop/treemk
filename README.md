# TreeMk - Markdown Editor

A wiki-Markdown text editor built with Qt, designed for organizing and
managing interconnected notes.

![screenshot](screenshot.png)

## Features

- File system tree view for note organization
- Wiki-style linking between notes with `[[note]]` syntax
- File inclusion with `[[!note]]` syntax - embeds content of linked file
  in preview
- Word prediction, based on the content of the current note
- Search and navigation tools
- Multi-tab editing
- Markdown preview with syntax highlighting
- LaTeX formula rendering (inline `$...$` and block `$$...$$`)
- Mermaid diagram rendering in preview
- Image embedding support
- Export to HTML, PDF, and DOCX (with Pandoc)

## Requirements

- Qt 6.2 or higher
- CMake 3.16 or higher
- C++17 compatible compiler
- md4c (for preview rendering)
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
- Mermaid.js is used for rendering diagrams.

### Optional: Mermaid Support in Exports

To render Mermaid diagrams when exporting to HTML, PDF, or DOCX with Pandoc,
you need to install `mermaid-filter`:

```bash
npm install -g mermaid-filter
```

If `mermaid-filter` is available in your PATH, it will be automatically used
during export. Otherwise, Mermaid code blocks will be exported as regular
code blocks.

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
