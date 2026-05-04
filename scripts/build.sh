#!/usr/bin/env bash
set -e

BUILD_DIR=build
ENABLE_COVERAGE=${ENABLE_COVERAGE:-OFF}

cmake -S . -B "$BUILD_DIR" -DENABLE_COVERAGE=$ENABLE_COVERAGE
cmake --build "$BUILD_DIR"