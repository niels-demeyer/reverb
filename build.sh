#!/usr/bin/env bash
set -e

BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --parallel "$(nproc)"

echo "Build complete: $BUILD_DIR/Reverb_artefacts/$BUILD_TYPE/VST3/Reverb.vst3"
