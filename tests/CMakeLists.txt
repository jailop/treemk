cmake_minimum_required(VERSION 3.16)

# Enable testing
enable_testing()

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Test Core Widgets WebEngineWidgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MD4C REQUIRED IMPORTED_TARGET md4c-html)

# Include parent directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Helper library for tests (shared code)
add_library(test_helpers STATIC
    test_markdown_helper.cpp
    test_markdown_helper.h
)

target_link_libraries(test_helpers
    Qt6::Core
    PkgConfig::MD4C
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

# Test 2: Full MarkdownPreview Integration Tests (requires GUI)
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
#     PkgConfig::MD4C
# )
# 
# add_test(NAME MarkdownPreview COMMAND test_markdownpreview)

# Output test results
set_tests_properties(MarkdownConversion PROPERTIES TIMEOUT 30)

# Add custom target to run all tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_markdown_conversion
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Tests configured. Run 'make check' to execute all tests.")
