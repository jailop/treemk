@echo off
REM TreeMk Windows Build and Deploy Script (Batch version)
REM Simple alternative to the PowerShell script

echo TreeMk Windows Deployment Script
echo =================================
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not found. Please install CMake and add it to PATH.
    pause
    exit /b 1
)

REM Set default Qt path - MODIFY THIS to match your Qt installation
set QT_PATH=C:\Qt\6.8.0\msvc2022_64
set BUILD_TYPE=Release
set OUTPUT_DIR=treemk-windows

REM Allow user to override Qt path
if "%1" NEQ "" set QT_PATH=%1

REM Check if Qt exists
if not exist "%QT_PATH%" (
    echo ERROR: Qt path not found: %QT_PATH%
    echo.
    echo Please edit this script and set QT_PATH to your Qt installation directory
    echo Or run: deploy-windows.bat "C:\Path\To\Qt\6.x.x\msvcXXXX_64"
    echo.
    pause
    exit /b 1
)

echo Qt Path: %QT_PATH%
echo Build Type: %BUILD_TYPE%
echo.

REM Clean and create build directory
set BUILD_DIR=build-release
if exist "%BUILD_DIR%" (
    echo Cleaning existing build directory...
    rmdir /s /q "%BUILD_DIR%"
)
mkdir "%BUILD_DIR%"

REM Configure with CMake
echo Configuring with CMake...
cd "%BUILD_DIR%"
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_PATH%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)

REM Build
echo.
echo Building project...
cmake --build . --config %BUILD_TYPE%

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

cd ..

REM Create output directory
if exist "%OUTPUT_DIR%" (
    echo Cleaning existing output directory...
    rmdir /s /q "%OUTPUT_DIR%"
)
mkdir "%OUTPUT_DIR%"

REM Copy executable
echo.
echo Copying executable...
copy "%BUILD_DIR%\%BUILD_TYPE%\treemk.exe" "%OUTPUT_DIR%\"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to copy executable
    pause
    exit /b 1
)

REM Deploy Qt dependencies
echo Deploying Qt dependencies...
cd "%OUTPUT_DIR%"
"%QT_PATH%\bin\windeployqt.exe" treemk.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: windeployqt failed
    cd ..
    pause
    exit /b 1
)

cd ..

REM Copy README
if exist "README.md" copy "README.md" "%OUTPUT_DIR%\"

REM Create distribution README
echo Creating README.txt...
(
echo TreeMk - Markdown Editor
echo ========================
echo.
echo Version: 0.1.0
echo.
echo ## Quick Start
echo.
echo 1. Run install.bat to install TreeMk to your system
echo    OR double-click treemk.exe to run directly
echo 2. Use File ^> Open Folder to open a directory with markdown files
echo 3. Start creating and editing your notes!
echo.
echo ## Features
echo.
echo - File system tree view for note organization
echo - Wiki-style linking between notes with [[note]] syntax
echo - File inclusion with [[!note]] syntax
echo - Markdown preview with syntax highlighting
echo - Image embedding and LaTeX formula rendering
echo - Multi-tab editing
echo - Export to HTML, PDF, and DOCX ^(requires Pandoc^)
echo.
echo ## Installing Pandoc ^(Optional^)
echo.
echo For export features, install Pandoc from:
echo https://pandoc.org/installing.html
echo.
echo Or via Chocolatey: choco install pandoc
echo.
echo ## More Information
echo.
echo See README.md for complete documentation
) > "%OUTPUT_DIR%\README.txt"

REM Create installer batch file
echo Creating installer...
(
echo @echo off
echo echo TreeMk Installer
echo echo ================
echo echo.
echo.
echo set "INSTALL_DIR=%%LOCALAPPDATA%%\TreeMk"
echo.
echo echo Installing TreeMk to: %%INSTALL_DIR%%
echo echo.
echo.
echo if exist "%%INSTALL_DIR%%" ^(
echo     echo Removing old installation...
echo     rmdir /s /q "%%INSTALL_DIR%%"
echo ^)
echo.
echo echo Creating installation directory...
echo mkdir "%%INSTALL_DIR%%"
echo.
echo echo Copying files...
echo xcopy /E /I /Y "%%~dp0*" "%%INSTALL_DIR%%"
echo.
echo echo Creating desktop shortcut...
echo powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%%USERPROFILE%%\Desktop\TreeMk.lnk'^); $SC.TargetPath = '%%INSTALL_DIR%%\treemk.exe'; $SC.WorkingDirectory = '%%INSTALL_DIR%%'; $SC.Save(^)"
echo.
echo echo Creating Start Menu shortcut...
echo if not exist "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\TreeMk" mkdir "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\TreeMk"
echo powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\TreeMk\TreeMk.lnk'^); $SC.TargetPath = '%%INSTALL_DIR%%\treemk.exe'; $SC.WorkingDirectory = '%%INSTALL_DIR%%'; $SC.Save(^)"
echo.
echo echo.
echo echo Installation complete!
echo echo.
echo echo TreeMk has been installed to: %%INSTALL_DIR%%
echo echo Desktop and Start Menu shortcuts have been created.
echo echo.
echo pause
) > "%OUTPUT_DIR%\install.bat"

REM Create uninstaller
echo Creating uninstaller...
(
echo @echo off
echo echo TreeMk Uninstaller
echo echo ==================
echo echo.
echo.
echo set "INSTALL_DIR=%%LOCALAPPDATA%%\TreeMk"
echo.
echo echo This will remove TreeMk from your system.
echo echo Installation directory: %%INSTALL_DIR%%
echo echo.
echo pause
echo.
echo if exist "%%INSTALL_DIR%%" ^(
echo     echo Removing installation directory...
echo     rmdir /s /q "%%INSTALL_DIR%%"
echo ^)
echo.
echo if exist "%%USERPROFILE%%\Desktop\TreeMk.lnk" ^(
echo     echo Removing desktop shortcut...
echo     del "%%USERPROFILE%%\Desktop\TreeMk.lnk"
echo ^)
echo.
echo if exist "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\TreeMk" ^(
echo     echo Removing Start Menu shortcuts...
echo     rmdir /s /q "%%APPDATA%%\Microsoft\Windows\Start Menu\Programs\TreeMk"
echo ^)
echo.
echo echo.
echo echo TreeMk has been uninstalled.
echo echo.
echo pause
) > "%OUTPUT_DIR%\uninstall.bat"

REM Done
echo.
echo ========================================
echo Deployment Complete!
echo ========================================
echo.
echo Output directory: %OUTPUT_DIR%
echo.
echo To distribute TreeMk:
echo 1. ZIP the %OUTPUT_DIR% folder
echo 2. Users can extract and run install.bat to install
echo 3. Or users can run treemk.exe directly
echo.
echo Files included:
echo - treemk.exe
echo - Qt libraries and dependencies
echo - install.bat / uninstall.bat
echo - README.txt
echo.
pause
