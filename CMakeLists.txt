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
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.treemk.app")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_ICON_FILE "treemk.icns")
endif()

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets
    WebEngineWidgets
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
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
