#!/bin/bash
# ============================================================
# MTS-CR-9000 Core Router — Yocto Build Script
# Autonomous build of mts-core-router-image
# ============================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
POKY_DIR="$HOME/poky"
BUILD_DIR="$POKY_DIR/build-mts"
LOG_FILE="$BUILD_DIR/build-$(date +%Y%m%d-%H%M%S).log"
MTS_LAYER="$PROJECT_DIR/linux/meta-mts"

echo "============================================"
echo " MTS-CR-9000 Yocto Build"
echo " Started: $(date)"
echo "============================================"

# ============================================================
# 1. Verify prerequisites
# ============================================================
echo "[1/6] Verifying prerequisites..."

if ! command -v python3 &>/dev/null; then
    echo "ERROR: python3 is required"
    exit 1
fi

if ! command -v git &>/dev/null; then
    echo "ERROR: git is required"
    exit 1
fi

if ! command -v tar &>/dev/null; then
    echo "ERROR: tar is required"
    exit 1
fi

if ! command -v bzip2 &>/dev/null; then
    echo "ERROR: bzip2 is required"
    exit 1
fi

if ! command -v unzip &>/dev/null; then
    echo "ERROR: unzip is required"
    exit 1
fi

# Verify Poky exists
if [ ! -d "$POKY_DIR" ]; then
    echo "[1/6] Cloning Poky..."
    git clone --depth 1 --branch kirkstone https://git.yoctoproject.org/git/poky.git "$POKY_DIR"
fi

# Verify meta-mts layer
if [ ! -d "$MTS_LAYER" ]; then
    echo "ERROR: meta-mts layer not found at $MTS_LAYER"
    exit 1
fi

# Verify bitbake
BITBAKE="$POKY_DIR/bitbake/bin/bitbake"
if [ ! -x "$BITBAKE" ]; then
    echo "ERROR: bitbake not found at $BITBAKE"
    exit 1
fi

echo "  [OK] All prerequisites verified"

# ============================================================
# 2. Create build environment
# ============================================================
echo "[2/6] Setting up build environment..."

mkdir -p "$BUILD_DIR"

# Source oe-init-build-env (create if not exists)
if [ ! -f "$BUILD_DIR/oe-init-build-env" ]; then
    cp "$POKY_DIR/oe-init-build-env" "$BUILD_DIR/oe-init-build-env"
    sed -i "s|OECORE_DEFAULT_SYSROOT.*||" "$BUILD_DIR/oe-init-build-env"
    sed -i "s|cd .*build.*|cd \"$BUILD_DIR\"|" "$BUILD_DIR/oe-init-build-env"
fi

# Initialize build env
source "$BUILD_DIR/oe-init-build-env" >/dev/null 2>&1 || true

# ============================================================
# 3. Configure bblayers.conf
# ============================================================
echo "[3/6] Configuring bblayers.conf..."

cat > "$BUILD_DIR/conf/bblayers.conf" <<'BBLAYERS_EOF'
# POKY_BBLAYERS_CONF_VERSION is increased each time build/conf/bblayers.conf
# changes incompatibly
POKY_BBLAYERS_CONF_VERSION = "2"

BBPATH = "${TOPDIR}"
BBFILES ?= ""

BBFILES += "${BSPDIR}/sources/poky/meta/recipes-*/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-*/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-core/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-core/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-devtools/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-devtools/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-extended/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-extended/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-filesystem/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-filesystem/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-kernel/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-kernel/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-multimedia/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-multimedia/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-graphics/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-graphics/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-connectivity/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-connectivity/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-networking/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-networking/*/*.bbappend \
            ${BSPDIR}/sources/poky/meta/recipes-support/*/*.bb \
            ${BSPDIR}/sources/poky/meta/recipes-support/*/*.bbappend"

BBFILES += "${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-*/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-*/*/*.bbappend \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-core/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-core/*/*.bbappend \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-extended/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-extended/*/*.bbappend \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-devtools/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-devtools/*/*.bbappend \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-networking/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-networking/*/*.bbappend \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-support/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-oe/recipes-support/*/*.bbappend"

BBFILES += "${BSPDIR}/sources/meta-openembedded/meta-networking/recipes-*/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-networking/recipes-*/*/*.bbappend"

BBFILES += "${BSPDIR}/sources/meta-openembedded/meta-python/recipes-*/*/*.bb \
            ${BSPDIR}/sources/meta-openembedded/meta-python/recipes-*/*/*.bbappend"

# MTS custom layer
BBLAYERS += "${PROJECT_DIR}/linux/meta-mts"
BBLAYERS_EOF

# Set BSPDIR and PROJECT_DIR
sed -i "s|\${BSPDIR}|$POKY_DIR|g" "$BUILD_DIR/conf/bblayers.conf"
sed -i "s|\${PROJECT_DIR}|$PROJECT_DIR|g" "$BUILD_DIR/conf/bblayers.conf"

echo "  [OK] bblayers.conf configured"

# ============================================================
# 4. Configure local.conf
# ============================================================
echo "[4/6] Configuring local.conf..."

cat > "$BUILD_DIR/conf/local.conf" <<'LOCALCONF_EOF'
# MTS-CR-9000 Build Configuration
# ============================================================

# Machine configuration
MACHINE = "mts-cr9000"

# License configuration
LICENSE_FLAGS_WHITELIST = "commercial-ml2"

# DL_DIR and SSTATE_DIR
DL_DIR ?= "${TOPDIR}/downloads"
SSTATE_DIR ?= "${TOPDIR}/sstate-cache"
TMPDIR = "${TOPDIR}/tmp"

# Package manager
PACKAGE_CLASSES = "package_rpm package_deb"

# Parallelism
PARALLEL_MAKE = "-j$(nproc)"
BB_NUMBER_THREADS = "$(nproc)"

# Kernel configuration
KERNEL_DEVICETREE = ""

# Image features
IMAGE_FSTYPES = "ext4 wic.gz"
IMAGE_ROOTFS_SIZE = "8388608"
IMAGE_ROOTFS_MAXSIZE = "16777216"
IMAGE_BOOT_FILES = "boot/vmlinuz boot/extlinux/extlinux.conf"

# Host tools
SDKMACHINE = "x86_64"

# Security
INHERIT += "rm_work"

# Debug
INHERIT += "debug-tasks"

# Network mirror (speed up downloads)
# PREMIRRORS = ""
# BB_GENERATE_MIRROR_TARBALLS = "1"

# Source directory
SOURCE_MIRROR_DIR ?= "${TOPDIR}/sourcedir"
UPSTREAM_MIRROR_DIR ?= "${TOPDIR}/sourcemirror"
BB_GENERATE_MIRROR_TARBALLS = "1"
CONFFILES_UPDATE = "1"
LOCALCONF_EOF

echo "  [OK] local.conf configured"

# ============================================================
# 5. Fetch dependencies
# ============================================================
echo "[5/6] Fetching dependencies..."

source "$BUILD_DIR/oe-init-build-env" >/dev/null 2>&1 || true

# Fetch required layers
cd "$POKY_DIR"
echo "  Fetching meta-openembedded..."
bitbake --version >/dev/null 2>&1 || true

# Clone meta-openembedded if not exists
if [ ! -d "$POKY_DIR/sources/meta-openembedded" ]; then
    echo "  [!] Cloning meta-openembedded..."
    git clone --depth 1 git://git.openembedded.org/meta-openembedded "$POKY_DIR/sources/meta-openembedded"
    cd "$POKY_DIR/sources/meta-openembedded"
    # Use kirkstone branch
    git checkout kirkstone 2>/dev/null || git checkout langdale 2>/dev/null || git checkout dunfell 2>/dev/null || true
    cd "$POKY_DIR"
fi

# Clone meta-virtualization for qemu
if [ ! -d "$POKY_DIR/sources/meta-virtualization" ]; then
    echo "  [!] Cloning meta-virtualization..."
    git clone --depth 1 git://git.yoctoproject.org/meta-virtualization "$POKY_DIR/sources/meta-virtualization" 2>/dev/null || \
    git clone --depth 1 https://github.com/meta-virtualization/meta-virtualization.git "$POKY_DIR/sources/meta-virtualization" 2>/dev/null || \
    echo "  [!] Could not clone meta-virtualization (optional)"
fi

# Clone meta-intel for x86-64 support
if [ ! -d "$POKY_DIR/sources/meta-intel" ]; then
    echo "  [!] Cloning meta-intel..."
    git clone --depth 1 git://git.yoctoproject.org/meta-intel "$POKY_DIR/sources/meta-intel" 2>/dev/null || \
    git clone --depth 1 https://github.com/meta-openembedded/meta-intel.git "$POKY_DIR/sources/meta-intel" 2>/dev/null || \
    echo "  [!] Could not clone meta-intel (optional)"
fi

# Clone meta-mingw for cross-compilation
if [ ! -d "$POKY_DIR/sources/meta-mingw" ]; then
    echo "  [!] Cloning meta-mingw..."
    git clone --depth 1 git://git.yoctoproject.org/meta-mingw "$POKY_DIR/sources/meta-mingw" 2>/dev/null || \
    git clone --depth 1 https://github.com/meta-openembedded/meta-mingw.git "$POKY_DIR/sources/meta-mingw" 2>/dev/null || \
    echo "  [!] Could not clone meta-mingw (optional)"
fi

# Clone meta-filesystems
if [ ! -d "$POKY_DIR/sources/meta-filesystems" ]; then
    echo "  [!] Cloning meta-filesystems..."
    git clone --depth 1 git://git.openembedded.org/meta-filesystems "$POKY_DIR/sources/meta-filesystems" 2>/dev/null || \
    git clone --depth 1 https://github.com/meta-openembedded/meta-filesystems.git "$POKY_DIR/sources/meta-filesystems" 2>/dev/null || \
    echo "  [!] Could not clone meta-filesystems (optional)"
fi

echo "  [OK] Dependencies fetched"

# ============================================================
# 6. Build image
# ============================================================
echo "[6/6] Building mts-core-router-image..."
echo "  Started: $(date)"
echo "  Log: $LOG_FILE"
echo "============================================"

# Run bitbake with logging
source "$BUILD_DIR/oe-init-build-env" >/dev/null 2>&1 || true

cd "$BUILD_DIR"

bitbake mts-core-router-image 2>&1 | tee "$LOG_FILE"

BUILD_EXIT=$?

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
    find "$BUILD_DIR/tmp/deploy/images/mts-cr9000" -type f 2>/dev/null | head -20 || true
    echo ""
    echo "Rootfs:"
    ls -lh "$BUILD_DIR/tmp/work/mts-cr9000-poky-linux/core-image-sato/1.0/temp/deploy/images/mts-cr9000/" 2>/dev/null | head -10 || true
else
    echo "[FAILURE] Build failed with exit code $BUILD_EXIT"
    echo "Check $LOG_FILE for details"
fi

exit $BUILD_EXIT
