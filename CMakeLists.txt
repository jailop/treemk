cmake_minimum_required(VERSION 3.16)

project(treemk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Platform-specific settings
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif()
if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "TreeMk")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "dev.datainquiry.treemk")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_ICON_FILE "treemk.icns")
endif()
if (UNIX AND NOT APPLE)
    add_compile_options(-Wall -g)
endif()

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets
    WebEngineWidgets
    Svg
)

# Find md4c library
if(WIN32)
    # On Windows, use vcpkg's find_package
    find_package(md4c CONFIG REQUIRED)
    set(MD4C_LIBRARIES md4c::md4c-html)
else()
    # On Unix systems, use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MD4C REQUIRED IMPORTED_TARGET md4c-html)
    set(MD4C_LIBRARIES PkgConfig::MD4C)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow_about.cpp
    src/mainwindow_actions.cpp
    src/tabeditor.cpp
    src/filesystemtreeview.cpp
    src/markdowneditor.cpp
    src/markdownhighlighter.cpp
    src/markdownpreview.cpp
    src/linkparser.cpp
    src/searchdialog.cpp
    src/settingsdialog.cpp
    src/formuladialog.cpp
    src/quickopendialog.cpp
    src/outlinepanel.cpp
    src/shortcutsdialog.cpp
    src/shortcutmanager.cpp
    src/thememanager.cpp
)

# Header files
set(HEADERS
    include/mainwindow.h
    include/tabeditor.h
    include/filesystemtreeview.h
    include/markdowneditor.h
    include/markdownhighlighter.h
    include/markdownpreview.h
    include/linkparser.h
    include/searchdialog.h
    include/settingsdialog.h
    include/formuladialog.h
    include/quickopendialog.h
    include/outlinepanel.h
    include/shortcutsdialog.h
    include/shortcutmanager.h
    include/thememanager.h
)

# Resource files
set(RESOURCES
    resources/resources.qrc
)

# Platform-specific executable configuration
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
elseif(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist.in"
    )
else()
    add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
endif()

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Svg
    ${MD4C_LIBRARIES}
)

# Platform-specific installation
if(WIN32)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .
    )
    # Deploy Qt dependencies on Windows
    if(Qt6_DIR)
        get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
        if(WINDEPLOYQT_EXECUTABLE)
            install(CODE "execute_process(COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe\")")
        endif()
    endif()
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
    )
    # Deploy Qt dependencies on macOS
    if(Qt6_DIR)
        get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
        if(MACDEPLOYQT_EXECUTABLE)
            install(CODE "execute_process(COMMAND \"${MACDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app\")")
        endif()
    endif()
else()
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )
endif()

# Enable testing
option(BUILD_TESTS "Build the test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# CPack Configuration for installers
set(CPACK_PACKAGE_NAME "TreeMk")
set(CPACK_PACKAGE_VENDOR "TreeMk")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Markdown editor with wiki-style linking")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TreeMk")

# Set license file if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
else()
    message(WARNING "LICENSE file not found - installer will not include license")
endif()

# Windows NSIS installer
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "TreeMk")
    set(CPACK_NSIS_PACKAGE_NAME "TreeMk")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/treemk.ico")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/treemk.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/treemk.ico")
    endif()
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\treemk.exe")
    set(CPACK_NSIS_MENU_LINKS
        "bin/treemk.exe" "TreeMk"
    )
endif()

# Linux DEB package
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TreeMk Developers")
    set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6widgets6, libqt6webenginewidgets6, libmd4c-html0")
    
    # Install desktop file if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/treemk.desktop")
        install(FILES resources/treemk.desktop
            DESTINATION share/applications)
    endif()
    
    # Install icon if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/treemk.png")
        install(FILES resources/treemk.png
            DESTINATION share/icons/hicolor/256x256/apps)
    endif()
endif()

include(CPack)

