cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(nrfbuttonclassification)

target_sources(app PRIVATE src/main.c)

# ============================================================================
# Edge Impulse Model Selection
# For testing ei-v2, comment out 'ei' and use 'ei-v2' instead
# Only ONE model can be active at a time due to symbol conflicts
# ============================================================================

# Original model (ei) - COMMENTED OUT FOR TESTING ei-v2
# add_subdirectory(ei)
# target_sources(app PRIVATE src/ei_glue.cpp)

# New model (ei-v2) - ACTIVE FOR TESTING
add_subdirectory(ei-v2)
target_sources(app PRIVATE src/ei_glue_v2.cpp)

# *** link minimal C++ runtime libs ***
# stdc++ provides new/delete, std::function, etc.
# supc++ provides low-level support (pure virtuals, atexit helpers on ARM, etc.)
zephyr_link_libraries(stdc++)
zephyr_link_libraries(supc++)

