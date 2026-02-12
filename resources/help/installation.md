# Installation

TreeMk is available for Linux, Windows, and macOS.

## Linux

### Binary Packages

Download from the [releases page](https://github.com/datainquiry/treemk/releases):
- `.deb` for Ubuntu/Debian
- `.rpm` for Fedora/RHEL
- `.pkg.tar.zst` for Arch Linux

### Building from Source

```bash
git clone https://github.com/datainquiry/treemk.git
cd treemk
mkdir build && cd build
cmake ..
make
sudo make install
```

## Requirements

- Qt 6.2 or later
- CMake 3.16+
- C++17 compiler

## First Run

On first launch, TreeMk will prompt you to select a workspace folder for your notes.

## Next Steps

See [Getting Started](getting-started.md) to begin using TreeMk.
