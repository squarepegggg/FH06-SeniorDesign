cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(nrfbuttonclassification)

target_sources(app PRIVATE src/main.c)

# Add the Edge Impulse library that lives in ./ei
add_subdirectory(ei)

# Add our tiny C++ glue file so C code can call the C++ classifier
target_sources(app PRIVATE src/ei_glue.cpp)

# *** NEW: link minimal C++ runtime libs ***
# stdc++ provides new/delete, std::function, etc.
# supc++ provides low-level support (pure virtuals, atexit helpers on ARM, etc.)
zephyr_link_libraries(stdc++)
zephyr_link_libraries(supc++)

