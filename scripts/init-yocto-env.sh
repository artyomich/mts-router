#!/bin/bash
# init-yocto-env.sh — Инициализация Yocto/Poky окружения для MTS Router
#
# Usage:
#   ./init-yocto-env.sh [--clean] [--poky-url <url>] [--branch <branch>] [--help]
#
# Автоматически клонирует poky, инициализирует окружение и добавляет meta-mts слой.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
META_MTS_DIR="${PROJECT_DIR}/linux/meta-mts"
WORK_DIR="${PROJECT_DIR}/build/yocto-work"
POKY_URL="${POKY_URL:-https://github.com/yoctoproject/poky.git}"
POKY_BRANCH="${POKY_BRANCH:-kirkstone}"
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

Initialize Yocto/Poky build environment for MTS Router devices.

Options:
  --clean                Remove existing poky and build directories
  --poky-url <url>       Custom poky repository URL
  --branch <branch>      Poky branch to use (default: kirkstone)
  --help                 Show this help message

Environment variables:
  POKY_URL               Override poky repository URL
  POKY_BRANCH            Override poky branch (kirkstone, langdale, morty)
  OE_INIT_PATH           Custom path for oe-init-build-env

Devices supported:
  mts-cr9000             Core Router (Yocto)
  mts-mc5000             Mobile Core (Yocto + K3s)

Examples:
  $(basename "$0")
  $(basename "$0") --clean
  POKY_BRANCH=langdale $(basename "$0")
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --poky-url)
            POKY_URL="$2"
            shift 2
            ;;
        --branch)
            POKY_BRANCH="$2"
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
    rm -rf "${WORK_DIR}/poky"
    rm -rf "${WORK_DIR}/build"
    log_success "Clean completed"
fi

# Step 2: Clone poky if not exists
if [ ! -d "${WORK_DIR}/poky/.git" ]; then
    log_info "Cloning poky from ${POKY_URL} (branch: ${POKY_BRANCH})..."
    mkdir -p "${WORK_DIR}"
    git clone --depth 1 --branch "${POKY_BRANCH}" "${POKY_URL}" "${WORK_DIR}/poky"
    log_success "Poky cloned successfully"
else
    log_info "Poky already exists, skipping clone"
fi

# Step 3: Clone meta-openembedded if not exists
if [ ! -d "${WORK_DIR}/meta-openembedded/.git" ]; then
    log_info "Cloning meta-openembedded..."
    git clone --depth 1 https://github.com/openembedded/meta-openembedded.git "${WORK_DIR}/meta-openembedded"
    log_success "meta-openembedded cloned"
else
    log_info "meta-openembedded already exists"
fi

# Step 4: Clone meta-virtualization if not exists
if [ ! -d "${WORK_DIR}/meta-virtualization/.git" ]; then
    log_info "Cloning meta-virtualization..."
    git clone --depth 1 https://github.com/openembedded/meta-virtualization.git "${WORK_DIR}/meta-virtualization"
    log_success "meta-virtualization cloned"
else
    log_info "meta-virtualization already exists"
fi

# Step 5: Clone meta-kubernetes for Mobile Core (K3s support)
if [ ! -d "${WORK_DIR}/meta-kubernetes/.git" ]; then
    log_info "Cloning meta-kubernetes..."
    git clone --depth 1 https://github.com/smpark/meta-kubernetes.git "${WORK_DIR}/meta-kubernetes" || \
        git clone --depth 1 https://github.com/meta-kubernetes/meta-kubernetes.git "${WORK_DIR}/meta-kubernetes"
    log_success "meta-kubernetes cloned"
else
    log_info "meta-kubernetes already exists"
fi

# Step 6: Verify meta-mts layer
if [ ! -d "${META_MTS_DIR}" ]; then
    log_error "meta-mts layer not found at ${META_MTS_DIR}"
    log_error "Please ensure linux/meta-mts/ directory exists"
    exit 1
fi

# Step 7: Create build directory structure
mkdir -p "${WORK_DIR}/build/cr9000"
mkdir -p "${WORK_DIR}/build/mc5000"
log_success "Build directories created"

# Step 8: Generate setup script for easy environment initialization
SETUP_SCRIPT="${WORK_DIR}/setup-env.sh"
cat > "${SETUP_SCRIPT}" <<'SETUP_EOF'
#!/bin/bash
# Auto-generated setup script for Yocto environment
# Source this script to initialize the build environment

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Initializing Yocto environment..."
echo "Poky: ${SCRIPT_DIR}/poky"
echo "Layers: meta-openembedded, meta-virtualization, meta-mts"

# Initialize Yocto build environment
source "${SCRIPT_DIR}/poky/oe-init-build-env" "${SCRIPT_DIR}/build/<device>"

# Add layers
bitbake-layers add-layer ../../meta-openembedded/meta-oe
bitbake-layers add-layer ../../meta-openembedded/meta-networking
bitbake-layers add-layer ../../meta-openembedded/meta-python
bitbake-layers add-layer ../../meta-virtualization
bitbake-layers add-layer ../../meta-mts

echo ""
echo "Environment ready. Available devices:"
echo "  mts-cr9000  - Core Router"
echo "  mts-mc5000  - Mobile Core"
echo ""
echo "To build:"
echo "  bitbake mts-core-router-image   # for mts-cr9000"
echo "  bitbake mts-mobile-core-image   # for mts-mc5000"
SETUP_EOF
chmod +x "${SETUP_SCRIPT}"
log_success "Setup script created at ${SETUP_SCRIPT}"

# Step 9: Display next steps
echo ""
echo "========================================================"
echo "  Yocto Environment Initialization Complete"
echo "========================================================"
echo ""
echo "Next steps:"
echo "  1. Source the environment: source ${SETUP_SCRIPT}"
echo "  2. For Core Router (CR-9000):"
echo "     bitbake mts-core-router-image"
echo "  3. For Mobile Core (MC-5000):"
echo "     bitbake mts-mobile-core-image"
echo ""
echo "Build artifacts will be in:"
echo "  ${WORK_DIR}/build/<device>/tmp/deploy/images/"
echo ""
