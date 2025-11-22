cmake_minimum_required(VERSION 3.16)

project(MkEd VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
)

# Header files
set(HEADERS
    include/mainwindow.h
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
)

# Create executable
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${HEADERS}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineWidgets
)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
