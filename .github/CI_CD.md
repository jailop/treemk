# TreeMk CI/CD - Multi-Platform Build System

This document explains how TreeMk automatically builds installers for macOS, Windows, and Linux using GitHub Actions.

## Overview

The GitHub Actions workflow automatically creates installers for all three major platforms whenever you push a version tag.

## Supported Platforms

- **macOS**: DMG installer (macOS 10.15+)
- **Windows**: NSIS installer (Windows 10+)
- **Linux**: AppImage (Ubuntu 20.04+)

## How to Trigger a Build

### Method 1: Create a Version Tag (Recommended)

```bash
# Create and push a version tag
git tag v0.1.0
git push origin v0.1.0
```

This will:
1. Build installers for all three platforms
2. Create a GitHub Release
3. Automatically upload all installers to the release

### Method 2: Manual Trigger

1. Go to your repository on GitHub
2. Click "Actions"
3. Select "Build Multi-Platform Installers"
4. Click "Run workflow"

This builds the installers but doesn't create a release.

## What Gets Built

### macOS (macos-latest runner)
- **Output**: `TreeMk-0.1.0.dmg`
- **Size**: ~30-50 MB
- **Features**:
  - Drag-and-drop installer
  - Includes Qt frameworks
  - Code-signed (if certificates are configured)

### Windows (windows-latest runner)
- **Output**: `TreeMk-0.1.0-Setup.exe`
- **Size**: ~40-60 MB
- **Features**:
  - NSIS installer wizard
  - Start menu shortcuts
  - Uninstaller included
  - All dependencies bundled

### Linux (ubuntu-20.04 runner)
- **Output**: `TreeMk-0.1.0-x86_64.AppImage`
- **Size**: ~80-100 MB
- **Features**:
  - Self-contained executable
  - No installation required
  - Works on most distributions

## Build Times

Approximate build times on GitHub Actions:
- macOS: 10-15 minutes
- Windows: 15-20 minutes  
- Linux: 10-15 minutes

Total: ~40-50 minutes for all platforms

## Artifacts

Each build produces artifacts that are available for 90 days:
- `TreeMk-macOS` - DMG file
- `TreeMk-Windows` - EXE installer
- `TreeMk-Linux` - AppImage file

## Requirements

### Repository Secrets

No secrets required for basic building. Optional for enhanced features:

- `APPLE_CERTIFICATE` - For code signing on macOS
- `APPLE_CERTIFICATE_PASSWORD` - Certificate password
- `APPLE_ID` - For notarization
- `APPLE_TEAM_ID` - Your Apple Team ID

### Repository Settings

1. Enable GitHub Actions in repository settings
2. Allow workflows to create releases (Settings → Actions → General)

## Customization

### Change Qt Version

Edit `.github/workflows/build-installers.yml`:

```yaml
- name: Install Qt
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.5.0'  # Change this
```

### Add Build Options

Add CMake flags:

```yaml
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
```

### Modify Installer Names

Edit the `DMG_NAME` in `create_dmg.sh`:

```bash
DMG_NAME="${APP_NAME}-${VERSION}"
```

## Testing Locally

### macOS
```bash
./create_dmg.sh
```

### Windows
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
cpack -G NSIS
```

### Linux
```bash
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
make install DESTDIR=AppDir
# Use linuxdeploy to create AppImage
```

## Troubleshooting

### Build Fails on macOS

**Issue**: Qt not found
**Solution**: Check Qt version is available in install-qt-action

**Issue**: md4c not found
**Solution**: Verify homebrew installation step

### Build Fails on Windows

**Issue**: MSVC not found
**Solution**: Ensure setup-msbuild action is running

**Issue**: md4c not found
**Solution**: Check vcpkg installation step

### Build Fails on Linux

**Issue**: Missing dependencies
**Solution**: Add missing packages to apt-get install list

**Issue**: AppImage creation fails
**Solution**: Check linuxdeploy output for missing libraries

## Workflow Status Badge

Add to your README.md:

```markdown
![Build Status](https://github.com/yourusername/treemk/workflows/Build%20Multi-Platform%20Installers/badge.svg)
```

## Release Process

1. Update version in `CMakeLists.txt`
2. Update `CHANGELOG.md`
3. Commit changes
4. Create and push tag:
   ```bash
   git tag -a v0.1.0 -m "Release version 0.1.0"
   git push origin v0.1.0
   ```
5. Wait for builds to complete (~40-50 minutes)
6. Check GitHub Releases for installers
7. Edit release notes if needed

## Cost

GitHub Actions provides 2,000 minutes/month free for private repos, unlimited for public repos. Each full build uses:
- macOS: ~15 minutes (10x multiplier = 150 minutes charged)
- Windows: ~20 minutes (2x multiplier = 40 minutes charged)
- Linux: ~15 minutes (1x multiplier = 15 minutes charged)

Total per release: ~205 minutes charged

## Advanced Features

### Matrix Builds

Build multiple Qt versions simultaneously:

```yaml
strategy:
  matrix:
    qt-version: [6.5.0, 6.6.0]
```

### Conditional Builds

Only build specific platforms:

```yaml
if: contains(github.event.head_commit.message, '[build-macos]')
```

### Caching

Speed up builds with caching:

```yaml
- name: Cache Qt
  uses: actions/cache@v3
  with:
    path: ~/Qt
    key: ${{ runner.os }}-Qt-${{ matrix.qt-version }}
```

## Support

For issues with the CI/CD workflow:
1. Check Actions tab for error logs
2. Review this documentation
3. Open an issue on GitHub

## License

Same as TreeMk project.
