cmake_minimum_required(VERSION 3.16)

# Enable testing
enable_testing()

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Test Core Widgets WebEngineWidgets)

# Find md4c library (platform-specific)
if(WIN32)
    find_package(md4c CONFIG REQUIRED)
    set(MD4C_LIBRARIES md4c::md4c-html)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MD4C REQUIRED IMPORTED_TARGET md4c-html)
    set(MD4C_LIBRARIES PkgConfig::MD4C)
endif()

# Include parent directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Business logic library (testable, no UI dependencies)
add_library(treemk_logic STATIC
    ${CMAKE_SOURCE_DIR}/include/managers/workspacemanager.h
    ${CMAKE_SOURCE_DIR}/src/logic/mainfilelocator.cpp
    ${CMAKE_SOURCE_DIR}/src/managers/workspacemanager.cpp
)

set_target_properties(treemk_logic PROPERTIES AUTOMOC ON)

target_link_libraries(treemk_logic
    Qt6::Core
)

# Window manager needs mainwindow for testing
add_library(treemk_windowmanager STATIC
    ${CMAKE_SOURCE_DIR}/include/managers/windowmanager.h
    ${CMAKE_SOURCE_DIR}/src/managers/windowmanager.cpp
)

set_target_properties(treemk_windowmanager PROPERTIES AUTOMOC ON)

target_link_libraries(treemk_windowmanager
    Qt6::Core
    Qt6::Widgets
)


# Helper library for tests (shared code)
add_library(test_helpers STATIC
    test_markdown_helper.cpp
    test_markdown_helper.h
)

target_link_libraries(test_helpers
    Qt6::Core
    ${MD4C_LIBRARIES}
)

# Test 1: Markdown Conversion Tests
add_executable(test_markdown_conversion
    test_markdown_conversion.cpp
)

target_link_libraries(test_markdown_conversion
    Qt6::Test
    Qt6::Core
    test_helpers
)

add_test(NAME MarkdownConversion COMMAND test_markdown_conversion)

# Test 2: MainFileLocator Tests
add_executable(test_mainfilelocator
    unit/test_mainfilelocator.cpp
)

target_link_libraries(test_mainfilelocator
    Qt6::Test
    Qt6::Core
    treemk_logic
)

add_test(NAME MainFileLocator COMMAND test_mainfilelocator)

# Test 3: WorkspaceManager Tests
add_executable(test_workspacemanager
    unit/test_workspacemanager.cpp
)

target_link_libraries(test_workspacemanager
    Qt6::Test
    Qt6::Core
    treemk_logic
)

add_test(NAME WorkspaceManager COMMAND test_workspacemanager)

# Test 4: WindowManager Tests  
add_executable(test_windowmanager
    unit/test_windowmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_about.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_actions.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_file.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_edit.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_export.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_tabs.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_view.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_ui.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow_insert.cpp
    ${CMAKE_SOURCE_DIR}/src/tabeditor.cpp
    ${CMAKE_SOURCE_DIR}/src/filesystemtreeview.cpp
    ${CMAKE_SOURCE_DIR}/src/markdowneditor.cpp
    ${CMAKE_SOURCE_DIR}/src/markdownhighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/markdownpreview.cpp
    ${CMAKE_SOURCE_DIR}/src/linkparser.cpp
    ${CMAKE_SOURCE_DIR}/src/searchdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/settingsdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/formuladialog.cpp
    ${CMAKE_SOURCE_DIR}/src/quickopendialog.cpp
    ${CMAKE_SOURCE_DIR}/src/outlinepanel.cpp
    ${CMAKE_SOURCE_DIR}/src/shortcutsdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/shortcutmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/thememanager.cpp
)

target_link_libraries(test_windowmanager
    Qt6::Test
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Svg
    treemk_windowmanager
    ${MD4C_LIBRARIES}
)

add_test(NAME WindowManager COMMAND test_windowmanager)

# Test 5: Full MarkdownPreview Integration Tests (requires GUI)
#
# Note: This test is disabled as it requires linking against the full
# application and dealing with Qt WebEngine initialization which is
# complex for unit tests.  The markdown conversion functionality is
# tested in test_markdown_conversion instead.

# add_executable(test_markdownpreview
#     test_markdownpreview.cpp
#     ${CMAKE_SOURCE_DIR}/src/markdownpreview.cpp
#     ${CMAKE_SOURCE_DIR}/src/markdownhighlighter.cpp
# )
# 
# target_link_libraries(test_markdownpreview
#     Qt6::Test
#     Qt6::Core
#     Qt6::Widgets
#     Qt6::WebEngineWidgets
#     ${MD4C_LIBRARIES}
# )
# 
# add_test(NAME MarkdownPreview COMMAND test_markdownpreview)

# Output test results
set_tests_properties(MarkdownConversion PROPERTIES TIMEOUT 30)
set_tests_properties(MainFileLocator PROPERTIES TIMEOUT 30)
set_tests_properties(WorkspaceManager PROPERTIES TIMEOUT 30)
set_tests_properties(WindowManager PROPERTIES TIMEOUT 30)

# Add custom target to run all tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_markdown_conversion test_mainfilelocator test_workspacemanager test_windowmanager
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Tests configured. Run 'make check' to execute all tests.")
