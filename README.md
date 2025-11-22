# MkEd - Markdown Editor

A feature-rich Markdown text editor built with Qt 6, designed for organizing and managing interconnected notes.

## Features

- File system tree view for note organization
- Wiki-style linking between notes
- Markdown preview with syntax highlighting
- Image embedding support
- LaTeX formula rendering
- And more...

## Requirements

- Qt 6.2 or higher
- CMake 3.16 or higher
- C++17 compatible compiler

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
./MkEd
```

### Windows

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust generator as needed)
cmake -G "MinGW Makefiles" ..
# or
cmake -G "Visual Studio 17 2022" ..

# Build
cmake --build .

# Run
./MkEd.exe
```

## Installation

After building:

```bash
cd build
sudo cmake --install .
```

## Development

### Project Structure

```
mked/
├── src/           # Source files (.cpp)
├── include/       # Header files (.h)
├── resources/     # Resources (icons, stylesheets, etc.)
├── docs/          # Documentation
├── tests/         # Test files
└── CMakeLists.txt # Build configuration
```

## License

[To be determined]

## Contributing

Contributions are welcome! Please read the contributing guidelines before submitting pull requests.

## Documentation

For more information, see:
- [Project Description](docs/description.md)
- [Development Tasks](docs/tasks.md)
