#!/bin/bash
set -e
APP_NAME="TreeMk"
VERSION="${VERSION:-0.6.0}"
BUILD_DIR="build"
DMG_NAME="${APP_NAME}-${VERSION}"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

if [ ! -d "${BUILD_DIR}/treemk.app" ]; then
    echo -e "${RED}Error: treemk.app not found in ${BUILD_DIR}${NC}"
    exit 1
fi
if command -v create-dmg &> /dev/null; then
    rm -f "${DMG_NAME}.dmg"
    TEMP_DIR="dmg_temp"
    rm -rf "${TEMP_DIR}"
    mkdir -p "${TEMP_DIR}"
    cp -R "${BUILD_DIR}/treemk.app" "${TEMP_DIR}/${APP_NAME}.app"
    cat > "${TEMP_DIR}/README.txt" << EOF
${APP_NAME} ${VERSION}
Installation Instructions:

1. Drag ${APP_NAME}.app to the Applications folder
2. Open ${APP_NAME} from your Applications folder
3. If you see a security warning, go to System Settings > Privacy & Security
   and click "Open Anyway"

Thank you for using ${APP_NAME}!

For more information, visit: https://github.com/yourusername/treemk
EOF
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
    rm -rf "${TEMP_DIR}"
    if [ -f "${DMG_NAME}.dmg" ]; then
        SIZE=$(du -h "${DMG_NAME}.dmg" | awk '{print $1}')
        echo -e "${GREEN}DMG created: ${DMG_NAME}.dmg${NC}"
        echo -e "${GREEN}Size: ${SIZE}${NC}"
        if [ -z "$CI" ]; then
            open -R "${DMG_NAME}.dmg"
        fi
    else
        echo -e "${RED}Error: DMG was not created${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}create-dmg not found.${NC}"
fi
