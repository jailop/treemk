#!/bin/bash
# Enhanced DMG creator using create-dmg tool (if available)
# Falls back to basic method if create-dmg is not installed

set -e

APP_NAME="TreeMk"
VERSION="0.1.0"
BUILD_DIR="build"
DMG_NAME="${APP_NAME}-${VERSION}"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== Building TreeMk DMG Installer (Enhanced) ===${NC}"

# Check if app exists
if [ ! -d "${BUILD_DIR}/treemk.app" ]; then
    echo -e "${RED}Error: treemk.app not found in ${BUILD_DIR}${NC}"
    exit 1
fi

# Check if create-dmg is installed
if command -v create-dmg &> /dev/null; then
    echo -e "${YELLOW}Using create-dmg tool...${NC}"
    
    # Clean up previous DMG
    rm -f "${DMG_NAME}.dmg"
    
    # Create DMG with create-dmg
    create-dmg \
        --volname "${APP_NAME} ${VERSION}" \
        --volicon "resources/treemk.icns" \
        --window-pos 200 120 \
        --window-size 640 400 \
        --icon-size 128 \
        --icon "${APP_NAME}.app" 160 180 \
        --hide-extension "${APP_NAME}.app" \
        --app-drop-link 480 180 \
        --no-internet-enable \
        "${DMG_NAME}.dmg" \
        "${BUILD_DIR}/treemk.app"
    
    echo -e "${GREEN}DMG created successfully: ${DMG_NAME}.dmg${NC}"
    open -R "${DMG_NAME}.dmg"
else
    echo -e "${YELLOW}create-dmg not found. Install it with:${NC}"
    echo -e "${YELLOW}brew install create-dmg${NC}"
    echo ""
    echo -e "${YELLOW}Falling back to basic DMG creation...${NC}"
    ./create_dmg.sh
fi
