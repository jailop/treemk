#!/bin/bash
# Script to create a DMG installer for TreeMk on macOS

set -e

# Configuration
APP_NAME="TreeMk"
VERSION="0.1.0"
BUILD_DIR="build"
DMG_NAME="${APP_NAME}-${VERSION}"
DMG_DIR="dmg_temp"
VOLUME_NAME="${APP_NAME} ${VERSION}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Building TreeMk DMG Installer ===${NC}"

# Check if app exists
if [ ! -d "${BUILD_DIR}/treemk.app" ]; then
    echo -e "${RED}Error: treemk.app not found in ${BUILD_DIR}${NC}"
    echo "Please build the application first with: cd build && cmake .. && make"
    exit 1
fi

# Clean up previous build
echo -e "${YELLOW}Cleaning up previous DMG build...${NC}"
rm -rf "${DMG_DIR}"
rm -f "${DMG_NAME}.dmg"
rm -f "${DMG_NAME}-temp.dmg"

# Create temporary directory for DMG contents
echo -e "${YELLOW}Creating DMG directory structure...${NC}"
mkdir -p "${DMG_DIR}"

# Copy the app bundle
echo -e "${YELLOW}Copying application bundle...${NC}"
cp -R "${BUILD_DIR}/treemk.app" "${DMG_DIR}/${APP_NAME}.app"

# Create Applications symlink
echo -e "${YELLOW}Creating Applications symlink...${NC}"
ln -s /Applications "${DMG_DIR}/Applications"

# Create a background image directory (optional)
mkdir -p "${DMG_DIR}/.background"

# Create a basic README
cat > "${DMG_DIR}/README.txt" << EOF
${APP_NAME} ${VERSION}

Installation Instructions:
==========================

1. Drag ${APP_NAME}.app to the Applications folder
2. Open ${APP_NAME} from your Applications folder
3. If you see a security warning, go to System Settings > Privacy & Security
   and click "Open Anyway"

Thank you for using ${APP_NAME}!

For more information, visit: https://github.com/yourusername/treemk
EOF

# Calculate size needed for DMG
echo -e "${YELLOW}Calculating DMG size...${NC}"
SIZE=$(du -sm "${DMG_DIR}" | awk '{print $1}')
SIZE=$((SIZE + 50)) # Add 50MB buffer

# Create temporary DMG
echo -e "${YELLOW}Creating temporary DMG...${NC}"
hdiutil create -srcfolder "${DMG_DIR}" \
    -volname "${VOLUME_NAME}" \
    -fs HFS+ \
    -fsargs "-c c=64,a=16,e=16" \
    -format UDRW \
    -size ${SIZE}m \
    "${DMG_NAME}-temp.dmg"

# Mount the temporary DMG
echo -e "${YELLOW}Mounting temporary DMG...${NC}"
MOUNT_DIR=$(hdiutil attach -readwrite -noverify -noautoopen "${DMG_NAME}-temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $3}')

if [ -z "$MOUNT_DIR" ]; then
    echo -e "${RED}Error: Failed to mount DMG${NC}"
    # Try to get the mount point another way
    MOUNT_DIR=$(hdiutil attach -readwrite -noverify -noautoopen "${DMG_NAME}-temp.dmg" | tail -1 | awk '{print $3}')
fi

echo -e "${YELLOW}Mounted at: ${MOUNT_DIR}${NC}"

if [ -z "$MOUNT_DIR" ]; then
    echo -e "${RED}Error: Could not determine mount point${NC}"
    exit 1
fi

# Set DMG window properties using AppleScript (skip in CI)
if [ -z "$CI" ]; then
    echo -e "${YELLOW}Setting DMG window properties...${NC}"
    cat > /tmp/dmg_setup.applescript << EOF
tell application "Finder"
    tell disk "${VOLUME_NAME}"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {100, 100, 740, 480}
        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 128
        set position of item "${APP_NAME}.app" of container window to {160, 180}
        set position of item "Applications" of container window to {480, 180}
        set position of item "README.txt" of container window to {320, 320}
        close
        open
        update without registering applications
        delay 2
    end tell
end tell
EOF

    osascript /tmp/dmg_setup.applescript || echo "Warning: Could not set DMG window properties"
    rm /tmp/dmg_setup.applescript
else
    echo -e "${YELLOW}Skipping GUI setup in CI environment${NC}"
fi

# Unmount the temporary DMG
echo -e "${YELLOW}Unmounting temporary DMG...${NC}"
sync
sync  # Double sync for safety
sleep 2  # Give system time to flush

# Try to detach with retries
for i in 1 2 3; do
    if hdiutil detach "${MOUNT_DIR}" 2>/dev/null; then
        break
    fi
    echo "Retry $i..."
    sleep 2
done

# Convert to compressed read-only DMG
echo -e "${YELLOW}Creating final compressed DMG...${NC}"
hdiutil convert "${DMG_NAME}-temp.dmg" \
    -format UDZO \
    -imagekey zlib-level=9 \
    -o "${DMG_NAME}.dmg"

# Clean up
echo -e "${YELLOW}Cleaning up temporary files...${NC}"
rm -rf "${DMG_DIR}"
rm -f "${DMG_NAME}-temp.dmg"

# Show result
if [ -f "${DMG_NAME}.dmg" ]; then
    SIZE=$(du -h "${DMG_NAME}.dmg" | awk '{print $1}')
    echo -e "${GREEN}==================================${NC}"
    echo -e "${GREEN}Success!${NC}"
    echo -e "${GREEN}DMG created: ${DMG_NAME}.dmg${NC}"
    echo -e "${GREEN}Size: ${SIZE}${NC}"
    echo -e "${GREEN}==================================${NC}"
    
    # Open Finder to show the DMG
    open -R "${DMG_NAME}.dmg"
else
    echo -e "${RED}Error: Failed to create DMG${NC}"
    exit 1
fi
