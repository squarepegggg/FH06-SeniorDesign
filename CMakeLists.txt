cmake_minimum_required(VERSION 3.13.1)

add_executable(app
    main.cpp
    edge-impulse-sdk/porting/posix/ei_classifier_porting.cpp
    edge-impulse-sdk/porting/posix/debug_log.cpp
)

if(NOT TARGET app)
     message(FATAL_ERROR "Please create a target named 'app' (ex: add_executable(app)) before adding this file")
endif()

include(edge-impulse-sdk/cmake/utils.cmake)
add_subdirectory(edge-impulse-sdk/cmake/zephyr)

RECURSIVE_FIND_FILE_APPEND(MODEL_SOURCE "tflite-model" "*.cpp")
target_include_directories(app PRIVATE .)

# add all sources to the project
target_sources(app PRIVATE ${MODEL_SOURCE})
