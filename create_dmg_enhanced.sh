#!/bin/bash
# Enhanced DMG creator using create-dmg tool (if available)
# Falls back to basic method if create-dmg is not installed
# Works in both local and CI environments

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
    echo -e "${YELLOW}Using create-dmg tool for professional DMG...${NC}"
    
    # Clean up previous DMG
    rm -f "${DMG_NAME}.dmg"
    
    # Create temporary directory with all content
    TEMP_DIR="dmg_temp"
    rm -rf "${TEMP_DIR}"
    mkdir -p "${TEMP_DIR}"
    
    # Copy app
    cp -R "${BUILD_DIR}/treemk.app" "${TEMP_DIR}/${APP_NAME}.app"
    
    # Create README
    cat > "${TEMP_DIR}/README.txt" << EOF
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
    
    # Create DMG with create-dmg
    # Suppress stderr in CI to avoid noise
    if [ -n "$CI" ]; then
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
            "${TEMP_DIR}" 2>/dev/null || {
                echo -e "${YELLOW}create-dmg had warnings (expected in CI), checking result...${NC}"
            }
    else
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
            "${TEMP_DIR}"
    fi
    
    # Clean up temp directory
    rm -rf "${TEMP_DIR}"
    
    # Verify DMG was created
    if [ -f "${DMG_NAME}.dmg" ]; then
        SIZE=$(du -h "${DMG_NAME}.dmg" | awk '{print $1}')
        echo -e "${GREEN}==================================${NC}"
        echo -e "${GREEN}Success!${NC}"
        echo -e "${GREEN}DMG created: ${DMG_NAME}.dmg${NC}"
        echo -e "${GREEN}Size: ${SIZE}${NC}"
        echo -e "${GREEN}==================================${NC}"
        
        # Open Finder to show the DMG (only locally)
        if [ -z "$CI" ]; then
            open -R "${DMG_NAME}.dmg"
        fi
    else
        echo -e "${RED}Error: DMG was not created${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}create-dmg not found.${NC}"
    if [ -z "$CI" ]; then
        echo -e "${YELLOW}Install it with: brew install create-dmg${NC}"
        echo ""
    fi
    echo -e "${YELLOW}Falling back to basic DMG creation...${NC}"
    ./create_dmg.sh
fi
