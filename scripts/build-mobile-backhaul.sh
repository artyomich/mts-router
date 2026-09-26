#!/bin/bash
# Build script for MTS-MB-3000 Mobile Backhaul API
# Builds existing C++ gRPC backend
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEVICE_DIR="${PROJECT_DIR}/mts-mb3000-api"
BUILD_DIR="${DEVICE_DIR}/build"

echo "[MB3000] Building MTS-MB-3000 Mobile Backhaul API..."

if [ ! -f "${DEVICE_DIR}/CMakeLists.txt" ]; then
    echo "[MB3000] ERROR: CMakeLists.txt not found in ${DEVICE_DIR}"
    exit 1
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "[MB3000] Running cmake..."
cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1

echo "[MB3000] Building..."
make -j"$(nproc)" 2>&1

if [ -f "${BUILD_DIR}/mts-mb3000-server" ]; then
    echo "[MB3000] Build successful: ${BUILD_DIR}/mts-mb3000-server"
    ls -lh "${BUILD_DIR}/mts-mb3000-server"
else
    echo "[MB3000] ERROR: Binary not found after build"
    exit 1
fi
