#!/bin/bash
# init-buildroot.sh — Инициализация Buildroot окружения для MTS Router
#
# Usage:
#   ./init-buildroot.sh [--clean] [--br-url <url>] [--branch <branch>] [--help]
#
# Автоматически клонирует Buildroot, настраивает BR2_EXTERNAL для meta-mts
# и создаёт конфигурацию для MTS-MB-3000 (S32G3).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
META_MTS_DIR="${PROJECT_DIR}/linux/meta-mts"
BUILDROOT_CONFIG="${PROJECT_DIR}/linux/buildroot/mts-s32g3-defconfig"
WORK_DIR="${PROJECT_DIR}/build/buildroot-work"
BR_URL="${BR_URL:-https://github.com/buildroot/buildroot.git}"
BR_BRANCH="${BR_BRANCH:-2024.05.x}"
CLEAN=false

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_success() { echo -e "${GREEN}[OK]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Initialize Buildroot build environment for MTS Router devices.

Options:
  --clean                Remove existing buildroot and build directories
  --br-url <url>         Custom buildroot repository URL
  --branch <branch>      Buildroot branch to use (default: 2024.05.x)
  --help                 Show this help message

Environment variables:
  BR_URL                 Override buildroot repository URL
  BR_BRANCH              Override buildroot branch

Devices supported:
  mts-mb3000             Mobile Backhaul (S32G3, Buildroot)

Examples:
  $(basename "$0")
  $(basename "$0") --clean
  BR_BRANCH=2024.02.x $(basename "$0")
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --br-url)
            BR_URL="$2"
            shift 2
            ;;
        --branch)
            BR_BRANCH="$2"
            shift 2
            ;;
        --help|-h)
            usage
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            ;;
    esac
done

# Step 1: Clean if requested
if [ "$CLEAN" = true ]; then
    log_info "Cleaning existing build environment..."
    rm -rf "${WORK_DIR}/buildroot"
    rm -rf "${WORK_DIR}/build"
    log_success "Clean completed"
fi

# Step 2: Clone buildroot if not exists
if [ ! -d "${WORK_DIR}/buildroot/.git" ]; then
    log_info "Cloning buildroot from ${BR_URL} (branch: ${BR_BRANCH})..."
    mkdir -p "${WORK_DIR}"
    git clone --depth 1 --branch "${BR_BRANCH}" "${BR_URL}" "${WORK_DIR}/buildroot"
    log_success "Buildroot cloned successfully"
else
    log_info "Buildroot already exists, skipping clone"
fi

# Step 3: Verify meta-mts layer exists
if [ ! -d "${META_MTS_DIR}" ]; then
    log_error "meta-mts layer not found at ${META_MTS_DIR}"
    exit 1
fi

# Step 4: Verify Buildroot defconfig exists
if [ ! -f "${BUILDROOT_CONFIG}" ]; then
    log_error "Buildroot defconfig not found at ${BUILDROOT_CONFIG}"
    exit 1
fi

# Step 5: Create build directory structure
mkdir -p "${WORK_DIR}/build/output"
log_success "Build directories created"

# Step 6: Generate setup script
SETUP_SCRIPT="${WORK_DIR}/setup-env.sh"
cat > "${SETUP_SCRIPT}" <<'SETUP_EOF'
#!/bin/bash
# Auto-generated setup script for Buildroot environment

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "${SCRIPT_DIR}")"

echo "Initializing Buildroot environment..."
echo "Buildroot: ${SCRIPT_DIR}/buildroot"
echo "BR2_EXTERNAL: ${SCRIPT_DIR}/../meta-mts"

cd "${SCRIPT_DIR}/buildroot"

# Copy defconfig
cp "${PROJECT_DIR}/linux/buildroot/mts-s32g3-defconfig" .config

# Build with BR2_EXTERNAL for meta-mts packages
echo ""
echo "To build MTS-MB-3000 image:"
echo "  make BR2_EXTERNAL=${PROJECT_DIR}/linux/meta-mts"
echo ""
echo "Output will be in:"
echo "  ${SCRIPT_DIR}/build/output/images/"
SETUP_EOF
chmod +x "${SETUP_SCRIPT}"
log_success "Setup script created at ${SETUP_SCRIPT}"

# Step 7: Create BR2_EXTERNAL package recipe for DPDK
DPDK_RECIPE_DIR="${META_MTS_DIR}/recipes-core/packages/dpdk"
mkdir -p "${DPDK_RECIPE_DIR}"
cat > "${DPDK_RECIPE_DIR}/dpdk-mts_24.07.bb" <<'DPDK_EOF'
require recipes-core/packages/dpdk-mts.inc

SUMMARY = "DPDK for MTS Router devices"
HOMEPAGE = "https://www.dpdk.org"
LICENSE = "DPDK"
LIC_FILES_CHKSUM = "file://LICENSE;md5=<license-hash>"

SRC_URI = "https://fast.dpdk.org/rel/dpdk-${PV}.tar.xz"
SRC_URI[sha256sum] = "<sha256-hash>"

S = "${WORKDIR}/dpdk-${PV}"

EXTRA_OEMAKE = "'PREFIX=${prefix}' 'LDFLAGS=${LDFLAGS}'"

do_configure() {
    cd ${S}
    meson configure \
        -Dlibdir=lib \
        -Denable_kmods=true \
        -Ddefault_library=static \
        -Dcpu_isa_include=all \
        ${@bb.utils.contains('DISTRO_FEATURES', 'pci', '-Dpci=true', '-Dpci=false', d)} \
        ${@bb.utils.contains('DISTRO_FEATURES', 'ioctls', '-Diova_as_va=true', '-Diova_as_va=false', d)}
}

do_compile() {
    cd ${S}
    meson compile -C build
}

do_install() {
    cd ${S}
    meson install -C build
}
DPDK_EOF
log_success "DPDK recipe created for BR2_EXTERNAL"

# Step 8: Display next steps
echo ""
echo "========================================================"
echo "  Buildroot Environment Initialization Complete"
echo "========================================================"
echo ""
echo "Next steps:"
echo "  1. Source the environment: source ${SETUP_SCRIPT}"
echo "  2. Build MTS-MB-3000 image:"
echo "     make BR2_EXTERNAL=${META_MTS_DIR}"
echo ""
echo "Build artifacts will be in:"
echo "  ${WORK_DIR}/build/output/images/"
echo ""
