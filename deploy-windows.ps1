# TreeMk Windows Deployment Script
# This script builds and packages TreeMk for Windows distribution

param(
    [string]$QtPath = "C:\Qt\6.8.0\msvc2022_64",
    [string]$BuildType = "Release",
    [string]$OutputDir = "treemk-windows"
)

# Error handling
$ErrorActionPreference = "Stop"

Write-Host "TreeMk Windows Deployment Script" -ForegroundColor Cyan
Write-Host "=================================" -ForegroundColor Cyan
Write-Host ""

# Check if CMake is available
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: CMake is not found. Please install CMake and add it to PATH." -ForegroundColor Red
    exit 1
}

# Check if Qt path exists
if (-not (Test-Path $QtPath)) {
    Write-Host "ERROR: Qt path not found: $QtPath" -ForegroundColor Red
    Write-Host "Please specify the correct Qt installation path using -QtPath parameter" -ForegroundColor Yellow
    Write-Host "Example: .\deploy-windows.ps1 -QtPath 'C:\Qt\6.8.0\msvc2022_64'" -ForegroundColor Yellow
    exit 1
}

# Find Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green
    }
} else {
    Write-Host "WARNING: Visual Studio not found automatically" -ForegroundColor Yellow
}

# Get the script directory (project root)
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

Write-Host "Project Root: $ProjectRoot" -ForegroundColor Green
Write-Host "Qt Path: $QtPath" -ForegroundColor Green
Write-Host "Build Type: $BuildType" -ForegroundColor Green
Write-Host ""

# Clean and create build directory
$BuildDir = Join-Path $ProjectRoot "build-release"
if (Test-Path $BuildDir) {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Cyan
Set-Location $BuildDir
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="$QtPath" -DCMAKE_BUILD_TYPE=$BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    exit 1
}

# Build the project
Write-Host ""
Write-Host "Building project..." -ForegroundColor Cyan
cmake --build . --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    exit 1
}

# Create output directory
Set-Location $ProjectRoot
$OutputPath = Join-Path $ProjectRoot $OutputDir
if (Test-Path $OutputPath) {
    Write-Host "Cleaning existing output directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $OutputPath
}
New-Item -ItemType Directory -Path $OutputPath | Out-Null

# Copy executable
Write-Host ""
Write-Host "Copying executable..." -ForegroundColor Cyan
$ExePath = Join-Path $BuildDir "$BuildType\treemk.exe"
if (-not (Test-Path $ExePath)) {
    Write-Host "ERROR: Executable not found at $ExePath" -ForegroundColor Red
    exit 1
}
Copy-Item $ExePath $OutputPath

# Deploy Qt dependencies
Write-Host "Deploying Qt dependencies..." -ForegroundColor Cyan
$WinDeployQt = Join-Path $QtPath "bin\windeployqt.exe"
if (-not (Test-Path $WinDeployQt)) {
    Write-Host "ERROR: windeployqt not found at $WinDeployQt" -ForegroundColor Red
    exit 1
}

Set-Location $OutputPath
& $WinDeployQt "treemk.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: windeployqt failed" -ForegroundColor Red
    exit 1
}

# Copy additional files
Write-Host "Copying additional files..." -ForegroundColor Cyan
Set-Location $ProjectRoot
Copy-Item "README.md" $OutputPath -ErrorAction SilentlyContinue

# Create README for distribution
$DistReadme = @"
TreeMk - Markdown Editor
========================

Version: 0.1.0

## Quick Start

1. Double-click treemk.exe to launch the application
2. Use File > Open Folder to open a directory containing your markdown files
3. Start creating and editing your notes!

## Features

- File system tree view for note organization
- Wiki-style linking between notes with [[note]] syntax
- File inclusion with [[!note]] syntax
- Markdown preview with syntax highlighting
- Image embedding support
- LaTeX formula rendering
- Multi-tab editing
- Export to HTML, PDF, and DOCX (requires Pandoc)

## Requirements

For export features (HTML, PDF, DOCX), you need to install Pandoc:
https://pandoc.org/installing.html

Or install via Chocolatey:
  choco install pandoc

## Documentation

For complete documentation, visit:
https://github.com/datainquiry/treemk

## License

See the README.md file for license information.
"@

$DistReadme | Out-File -FilePath (Join-Path $OutputPath "README.txt") -Encoding UTF8

# Create installer script
$InstallerScript = @"
@echo off
echo TreeMk Installer
echo ================
echo.

set "INSTALL_DIR=%LOCALAPPDATA%\TreeMk"

echo Installing TreeMk to: %INSTALL_DIR%
echo.

if exist "%INSTALL_DIR%" (
    echo Removing old installation...
    rmdir /s /q "%INSTALL_DIR%"
)

echo Creating installation directory...
mkdir "%INSTALL_DIR%"

echo Copying files...
xcopy /E /I /Y "%~dp0*" "%INSTALL_DIR%"

echo Creating desktop shortcut...
powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%USERPROFILE%\Desktop\TreeMk.lnk'); $SC.TargetPath = '%INSTALL_DIR%\treemk.exe'; $SC.WorkingDirectory = '%INSTALL_DIR%'; $SC.Save()"

echo Creating Start Menu shortcut...
if not exist "%APPDATA%\Microsoft\Windows\Start Menu\Programs\TreeMk" mkdir "%APPDATA%\Microsoft\Windows\Start Menu\Programs\TreeMk"
powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%APPDATA%\Microsoft\Windows\Start Menu\Programs\TreeMk\TreeMk.lnk'); $SC.TargetPath = '%INSTALL_DIR%\treemk.exe'; $SC.WorkingDirectory = '%INSTALL_DIR%'; $SC.Save()"

echo.
echo Installation complete!
echo.
echo TreeMk has been installed to: %INSTALL_DIR%
echo Desktop and Start Menu shortcuts have been created.
echo.
pause
"@

$InstallerScript | Out-File -FilePath (Join-Path $OutputPath "install.bat") -Encoding ASCII

# Create uninstaller script
$UninstallerScript = @"
@echo off
echo TreeMk Uninstaller
echo ==================
echo.

set "INSTALL_DIR=%LOCALAPPDATA%\TreeMk"

echo This will remove TreeMk from your system.
echo Installation directory: %INSTALL_DIR%
echo.
pause

if exist "%INSTALL_DIR%" (
    echo Removing installation directory...
    rmdir /s /q "%INSTALL_DIR%"
)

if exist "%USERPROFILE%\Desktop\TreeMk.lnk" (
    echo Removing desktop shortcut...
    del "%USERPROFILE%\Desktop\TreeMk.lnk"
)

if exist "%APPDATA%\Microsoft\Windows\Start Menu\Programs\TreeMk" (
    echo Removing Start Menu shortcuts...
    rmdir /s /q "%APPDATA%\Microsoft\Windows\Start Menu\Programs\TreeMk"
)

echo.
echo TreeMk has been uninstalled.
echo.
pause
"@

$UninstallerScript | Out-File -FilePath (Join-Path $OutputPath "uninstall.bat") -Encoding ASCII

# Create a ZIP archive if possible
Write-Host ""
Write-Host "Creating ZIP archive..." -ForegroundColor Cyan
$ZipPath = Join-Path $ProjectRoot "$OutputDir.zip"
if (Test-Path $ZipPath) {
    Remove-Item $ZipPath
}

try {
    Compress-Archive -Path $OutputPath -DestinationPath $ZipPath
    Write-Host "ZIP archive created: $ZipPath" -ForegroundColor Green
} catch {
    Write-Host "WARNING: Could not create ZIP archive" -ForegroundColor Yellow
}

# Summary
Write-Host ""
Write-Host "Deployment Complete!" -ForegroundColor Green
Write-Host "===================" -ForegroundColor Green
Write-Host "Output directory: $OutputPath" -ForegroundColor Green
Write-Host ""
Write-Host "To distribute TreeMk:" -ForegroundColor Cyan
Write-Host "1. Share the ZIP file: $OutputDir.zip" -ForegroundColor White
Write-Host "2. Users can extract and run install.bat to install" -ForegroundColor White
Write-Host "3. Or users can run treemk.exe directly from the extracted folder" -ForegroundColor White
Write-Host ""
Write-Host "Files included:" -ForegroundColor Cyan
Write-Host "- treemk.exe (application)" -ForegroundColor White
Write-Host "- Qt libraries and dependencies" -ForegroundColor White
Write-Host "- install.bat (installer script)" -ForegroundColor White
Write-Host "- uninstall.bat (uninstaller script)" -ForegroundColor White
Write-Host "- README.txt (user documentation)" -ForegroundColor White
Write-Host ""
