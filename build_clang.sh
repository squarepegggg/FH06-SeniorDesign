#!/bin/bash
# Build script for Edge Impulse classification demo using clang

set -e

# Find all source files
SOURCES=(
    main.cpp
    edge-impulse-sdk/porting/posix/ei_classifier_porting.cpp
    edge-impulse-sdk/porting/posix/debug_log.cpp
    tflite-model/tflite_learn_810907_3_compiled.cpp
)

# Add all C++ files from edge-impulse-sdk (excluding porting folders we don't need)
while IFS= read -r file; do
    # Skip other porting implementations
    if [[ "$file" != *"porting/"* ]] || [[ "$file" == *"porting/posix"* ]]; then
        SOURCES+=("$file")
    fi
done < <(find edge-impulse-sdk -name "*.cpp" -o -name "*.cc" | grep -v "porting/" | grep -v "/porting/")

# Add TensorFlow Lite common.c
SOURCES+=("edge-impulse-sdk/tensorflow/lite/c/common.c")

# Compile flags
CXXFLAGS="-std=c++11 -I. -O2"

# Compile all sources to object files first (for faster iteration)
echo "Compiling ${#SOURCES[@]} source files..."
OBJS=()
for src in "${SOURCES[@]}"; do
    obj="${src//\//_}.o"
    obj="${obj//./_}"
    OBJS+=("$obj")
    echo "  $src -> $obj"
    clang++ $CXXFLAGS -c "$src" -o "$obj" || {
        echo "Failed to compile $src"
        # Clean up on error
        rm -f "${OBJS[@]}"
        exit 1
    }
done

# Link everything
echo "Linking..."
clang++ "${OBJS[@]}" -o app

# Clean up object files
rm -f "${OBJS[@]}"

echo "Build complete: ./app"

