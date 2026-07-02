#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
ORDERS="${ORDERS:-1000000}"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc)"

mkdir -p results
"$BUILD_DIR/fastexchange" benchmark --scenario scenarios/balanced.yaml --orders "$ORDERS"
"$BUILD_DIR/fastexchange" benchmark --scenario scenarios/flash-crash.yaml --orders "$ORDERS"
"$BUILD_DIR/fastexchange" compare results/benchmark-balanced-metrics.json results/benchmark-flash-crash-metrics.json
