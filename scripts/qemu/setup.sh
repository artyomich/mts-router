#!/bin/bash
# setup.sh — Настройка QEMU окружения для тестирования MTS Router образов
#
# Usage:
#   ./setup.sh [--clean] [--help]
#
# Устанавливает QEMU с необходимыми targets и настраивает сетевые интерфейсы.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
QEMU_DIR="${BUILD_DIR}/qemu-work"
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

Setup QEMU environment for MTS Router image testing.

Options:
  --clean                Remove existing QEMU work directory
  --help                 Show this help message

Supported architectures:
  aarch64                MTS-MB-3000, MTS-MC-5000 (virt machine)
  mips64el               MTS-OLT-2000 (malta machine)
  arm                    MTS-ER-1000, MTS-RG-500 (versatilepb machine)

Examples:
  $(basename "$0")
  $(basename "$0") --clean
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
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
    log_info "Cleaning existing QEMU work directory..."
    rm -rf "${QEMU_DIR}"
    log_success "Clean completed"
fi

# Step 2: Create work directories
mkdir -p "${QEMU_DIR}/{aarch64,mips64el,arm}"
mkdir -p "${BUILD_DIR}/test-results/qemu"
log_success "QEMU work directories created"

# Step 3: Check QEMU installation
log_info "Checking QEMU installation..."
QEMU_TARGETS=""

if command -v qemu-system-aarch64 &>/dev/null; then
    QEMU_TARGETS="${QEMU_TARGETS} aarch64"
    log_success "qemu-system-aarch64 found: $(qemu-system-aarch64 --version)"
else
    log_warn "qemu-system-aarch64 not found"
fi

if command -v qemu-system-mips64el &>/dev/null; then
    QEMU_TARGETS="${QEMU_TARGETS} mips64el"
    log_success "qemu-system-mips64el found: $(qemu-system-mips64el --version)"
else
    log_warn "qemu-system-mips64el not found"
fi

if command -v qemu-system-arm &>/dev/null; then
    QEMU_TARGETS="${QEMU_TARGETS} arm"
    log_success "qemu-system-arm found: $(qemu-system-arm --version)"
else
    log_warn "qemu-system-arm not found"
fi

# Step 4: Install QEMU if missing
if [ -z "${QEMU_TARGETS}" ]; then
    log_info "Installing QEMU with all targets..."
    if command -v apt-get &>/dev/null; then
        sudo apt-get update
        sudo apt-get install -y qemu-system-arm qemu-system-mips qemu-system-aarch64 qemu-utils
    elif command -v yum &>/dev/null; then
        sudo yum install -y qemu-system-arm qemu-system-mips qemu-system-aarch64 qemu-img
    elif command -v dnf &>/dev/null; then
        sudo dnf install -y qemu-system-arm qemu-system-mips qemu-system-aarch64 qemu-img
    else
        log_error "Package manager not supported. Install QEMU manually:"
        log_error "  Ubuntu/Debian: apt-get install qemu-system-arm qemu-system-mips qemu-system-aarch64"
        log_error "  Fedora/RHEL: yum install qemu-system-arm qemu-system-mips qemu-system-aarch64"
        log_error "  Arch: pacman -S qemu-full"
        exit 1
    fi
    log_success "QEMU installed successfully"
fi

# Step 5: Setup network tap interface
log_info "Setting up network tap interface..."
if ip link show mts-tap0 &>/dev/null; then
    log_success "Tap interface mts-tap0 already exists"
else
    if sudo ip tuntap add dev mts-tap0 mode tap 2>/dev/null; then
        sudo ip link set mts-tap0 up 2>/dev/null || true
        sudo ip addr add 10.255.255.1/24 dev mts-tap0 2>/dev/null || true
        log_success "Tap interface mts-tap0 created"
    else
        log_warn "Could not create tap interface - running without network"
    fi
fi

# Step 6: Generate test configuration
TEST_CONFIG="${QEMU_DIR}/qemu-test-config.json"
cat > "${TEST_CONFIG}" <<EOF
{
  "aarch64": {
    "qemu_binary": "qemu-system-aarch64",
    "machine": "virt",
    "cpu": "cortex-a72",
    "memory": "2048",
    "kernel_args": "console=ttyAMA0",
    "network_device": "virtio-net-device",
    "netdev": "user,id=n1",
    "test_devices": ["mts-mb3000", "mts-mc5000"]
  },
  "mips64el": {
    "qemu_binary": "qemu-system-mips64el",
    "machine": "malta",
    "cpu": "MIPS64",
    "memory": "1024",
    "kernel_args": "console=ttyS0",
    "network_device": "e1000",
    "netdev": "user,id=n1",
    "test_devices": ["mts-olt2000"]
  },
  "arm": {
    "qemu_binary": "qemu-system-arm",
    "machine": "versatilepb",
    "cpu": "arm1176",
    "memory": "512",
    "kernel_args": "console=ttyAMA0",
    "network_device": "rtl8139",
    "netdev": "user,id=n1",
    "test_devices": ["mts-er1000", "mts-rg500"]
  }
}
EOF
log_success "Test configuration created at ${TEST_CONFIG}"

# Step 7: Display summary
echo ""
echo "========================================================"
echo "  QEMU Environment Setup Complete"
echo "========================================================"
echo ""
echo "Installed targets:${QEMU_TARGETS}"
echo "Work directory: ${QEMU_DIR}"
echo "Test results: ${BUILD_DIR}/test-results/qemu/"
echo ""
echo "Next steps:"
echo "  1. Place kernel and rootfs images in the appropriate architecture directory"
echo "  2. Run tests: scripts/qemu/run-test.sh --arch <arch> --image <path>"
echo "  3. Run all tests: scripts/qemu/test-all.sh"
echo ""
