# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(button LANGUAGES C CXX)

# Set C++ standard (required by Edge Impulse library)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add C and C++ source files
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/ei_classifier_wrapper.cpp)

# Add include directory for wrapper header
target_include_directories(app PRIVATE src)

# Add classification-task-demo library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../classification-task-demo-cpp-mcu-v1 ${CMAKE_CURRENT_BINARY_DIR}/classification-task-demo)
