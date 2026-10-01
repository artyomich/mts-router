#!/bin/bash
# ============================================================
# Wrapper script to run bitbake for MTS-CR-9000
# ============================================================

POKY_DIR="$HOME/poky"
BUILD_DIR="$POKY_DIR/build-mts"
LOG_FILE="$BUILD_DIR/build-$(date +%Y%m%d-%H%M%S).log"

echo "============================================"
echo " MTS-CR-9000 Bitbake Build"
echo " Started: $(date)"
echo " Log: $LOG_FILE"
echo "============================================"

# Initialize build environment (source it!)
cd "$BUILD_DIR"

# Export required variables for oe-init-build-env compatibility
export ZSH_NAME=""
export BBSERVER=""

. "$POKY_DIR/oe-init-build-env" .

echo ""
echo "Build environment initialized"
echo "MACHINE = $MACHINE"
echo "BBPATH = $BBPATH"
echo "BSPDIR = $BSPDIR"
echo "OEROOT = $OEROOT"
echo ""

# Verify bitbake is available
if ! command -v bitbake &>/dev/null; then
    echo "ERROR: bitbake not found in PATH"
    exit 1
fi

echo "Bitbake version: $(bitbake --version 2>&1 | head -3)"
echo ""

# Run bitbake
echo "Starting bitbake mts-core-router-image..."
bitbake mts-core-router-image 2>&1 | tee "$LOG_FILE"

BUILD_EXIT=${PIPESTATUS[0]}

echo ""
echo "============================================"
echo " Build completed: $(date)"
echo " Exit code: $BUILD_EXIT"
echo " Log: $LOG_FILE"
echo "============================================"

if [ $BUILD_EXIT -eq 0 ]; then
    echo "[SUCCESS] Image built successfully!"
    echo ""
    echo "Artifacts:"
    find "$BUILD_DIR/tmp/deploy/images/mts-cr9000" -type f 2>/dev/null | head -30 || true
else
    echo "[FAILURE] Build failed with exit code $BUILD_EXIT"
    echo "Check $LOG_FILE for details"
fi

exit $BUILD_EXIT
