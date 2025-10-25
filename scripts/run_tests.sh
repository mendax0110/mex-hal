#!/usr/bin/env bash
set -e
set -o pipefail

BUILD_DIR="build"
BUILD_TYPE="Debug"

echo "========================================"
echo "Configuring and building mex-hal"
echo "========================================"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..
cmake --build . --config "$BUILD_TYPE" -- -j$(nproc)

echo "========================================"
echo "Running ctest executables"
echo "========================================"
ctest --output-on-failure -V --progress --tests-regex ".*" --test-output-format=google-test

echo "========================================"
echo "All tests completed successfully"
echo "========================================"
