cmake_minimum_required(VERSION 3.24)
project(GridLiberty LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type defaults to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

# Sub-projects
add_subdirectory(engine)

# Testing
enable_testing()
add_subdirectory(engine/tests)

# macOS-only: UI and Python bridge
if(APPLE)
    enable_language(OBJC)
    set(CMAKE_OBJC_STANDARD 11)

    find_library(COCOA_FW Cocoa)
    find_library(QUARTZ_FW QuartzCore)
    find_package(Python3 3.10 COMPONENTS Development)

    add_subdirectory(python)
    add_subdirectory(ui)
endif()
