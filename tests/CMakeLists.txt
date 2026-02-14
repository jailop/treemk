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
    ${CMAKE_SOURCE_DIR}/include/logic/systemprompts.h
    ${CMAKE_SOURCE_DIR}/src/logic/mainfilelocator.cpp
    ${CMAKE_SOURCE_DIR}/src/logic/systemprompts.cpp
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

# Test 4: LinkParser Tests
add_executable(test_linkparser
    unit/test_linkparser.cpp
    ${CMAKE_SOURCE_DIR}/include/linkparser.h
    ${CMAKE_SOURCE_DIR}/src/linkparser.cpp
    ${CMAKE_SOURCE_DIR}/include/backlinks/backlinksmanager.h
    ${CMAKE_SOURCE_DIR}/src/backlinks/backlinksmanager.cpp
)

set_target_properties(test_linkparser PROPERTIES AUTOMOC ON)

target_link_libraries(test_linkparser
    Qt6::Test
    Qt6::Core
)

add_test(NAME LinkParser COMMAND test_linkparser)

# Test 5: AIAssist Dialog Tests
add_executable(test_aiassist_dialog
    unit/test_aiassist_dialog.cpp
    ${CMAKE_SOURCE_DIR}/include/aiassistdialog.h
    ${CMAKE_SOURCE_DIR}/include/logic/aiprovider.h
    ${CMAKE_SOURCE_DIR}/include/logic/ollamaprovider.h
    ${CMAKE_SOURCE_DIR}/include/logic/openaiprovider.h
    ${CMAKE_SOURCE_DIR}/include/logic/systemprompts.h
    ${CMAKE_SOURCE_DIR}/src/aiassistant/aiassistdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/logic/aiprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/logic/ollamaprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/logic/openaiprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/logic/systemprompts.cpp
)

set_target_properties(test_aiassist_dialog PROPERTIES AUTOMOC ON AUTOUIC ON)

target_link_libraries(test_aiassist_dialog
    Qt6::Test
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

add_test(NAME AIAssistDialog COMMAND test_aiassist_dialog)

# Note: WindowManager unit tests require MainWindow, will be tested via integration

# Integration Tests
# Collect all main application sources and headers
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
# Remove main.cpp as test has its own main
list(FILTER INTEGRATION_TEST_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

file(GLOB_RECURSE INTEGRATION_TEST_HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

add_executable(test_integration
    integration/test_integration.cpp
    ${INTEGRATION_TEST_SOURCES}
    ${INTEGRATION_TEST_HEADERS}
)

set_target_properties(test_integration PROPERTIES AUTOMOC ON AUTOUIC ON)

target_link_libraries(test_integration
    Qt6::Test
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Svg
    Qt6::Network
    treemk_windowmanager
    treemk_logic
    ${MD4C_LIBRARIES}
)

add_test(NAME Integration COMMAND test_integration)

# Test 4: Full MarkdownPreview Integration Tests (requires GUI)
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
set_tests_properties(LinkParser PROPERTIES TIMEOUT 30)
set_tests_properties(AIAssistDialog PROPERTIES TIMEOUT 30)
set_tests_properties(Integration PROPERTIES TIMEOUT 30)

# Add custom target to run all tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_markdown_conversion test_mainfilelocator test_workspacemanager test_linkparser test_aiassist_dialog test_integration
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Tests configured. Run 'make check' to execute all tests.")
