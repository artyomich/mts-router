#!/bin/bash
# run-test.sh — Запуск QEMU тестирования для одного образа MTS Router
#
# Usage:
#   ./run-test.sh --arch <aarch64|mips64el|arm> --image <path-to-image> [--kernel <path>] [--rootfs <path>] [--timeout <seconds>] [--verify <script>] [--help]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
QEMU_DIR="${BUILD_DIR}/qemu-work"
TEST_RESULTS_DIR="${BUILD_DIR}/test-results/qemu"
LOG_DIR="${TEST_RESULTS_DIR}"

ARCH=""
IMAGE=""
KERNEL=""
ROOTFS=""
TIMEOUT=300
VERIFY_SCRIPT=""
VERBOSE=false

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

Run QEMU boot test for a single MTS Router image.

Required options:
  --arch <arch>          Architecture (aarch64, mips64el, arm)
  --image <path>         Path to kernel or combined image

Optional options:
  --kernel <path>        Path to kernel image (zImage, Image, uImage)
  --rootfs <path>        Path to rootfs (ext4, cpio.gz, tar)
  --dtb <path>           Path to device tree blob
  --timeout <seconds>    Test timeout in seconds (default: 300)
  --verify <script>      Verification script to run after boot
  --verbose              Enable verbose output
  --help                 Show this help message

Examples:
  $(basename "$0") --arch aarch64 --kernel ./uImage --rootfs ./rootfs.cpio.gz
  $(basename "$0") --arch mips64el --image ./vmlinux-initrd.bin --timeout 120
  $(basename "$0") --arch arm --kernel ./zImage --rootfs ./rootfs.ext4 --verify ./verify.sh
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --arch)
            ARCH="$2"
            shift 2
            ;;
        --image)
            IMAGE="$2"
            shift 2
            ;;
        --kernel)
            KERNEL="$2"
            shift 2
            ;;
        --rootfs)
            ROOTFS="$2"
            shift 2
            ;;
        --dtb)
            DTB="$2"
            shift 2
            ;;
        --timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        --verify)
            VERIFY_SCRIPT="$2"
            shift 2
            ;;
        --verbose|-v)
            VERBOSE=true
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

# Validate required parameters
if [ -z "${ARCH}" ]; then
    log_error "Architecture is required (--arch)"
    exit 1
fi

if [ -z "${IMAGE}" ] && [ -z "${KERNEL}" ]; then
    log_error "Image or kernel is required (--image or --kernel)"
    exit 1
fi

# Create test-specific directories
TEST_DIR="${TEST_RESULTS_DIR}/${ARCH}"
mkdir -p "${TEST_DIR}"
LOG_FILE="${TEST_DIR}/boot-$(date +%Y%m%d-%H%M%S).log"

# Determine QEMU binary and configuration
case "${ARCH}" in
    aarch64)
        QEMU_BIN="qemu-system-aarch64"
        MACHINE="virt"
        CPU="cortex-a72"
        MEMORY="2048"
        KERNEL_ARGS="console=ttyAMA0"
        NET_DEV="virtio-net-device"
        ;;
    mips64el)
        QEMU_BIN="qemu-system-mips64el"
        MACHINE="malta"
        CPU="MIPS64"
        MEMORY="1024"
        KERNEL_ARGS="console=ttyS0"
        NET_DEV="e1000"
        ;;
    arm)
        QEMU_BIN="qemu-system-arm"
        MACHINE="versatilepb"
        CPU="arm1176"
        MEMORY="512"
        KERNEL_ARGS="console=ttyAMA0"
        NET_DEV="rtl8139"
        ;;
    *)
        log_error "Unsupported architecture: ${ARCH}"
        exit 1
        ;;
esac

# Check QEMU binary exists
if ! command -v "${QEMU_BIN}" &>/dev/null; then
    log_error "QEMU binary not found: ${QEMU_BIN}"
    log_error "Install QEMU: apt-get install qemu-system-arm qemu-system-aarch64 qemu-system-mips"
    exit 1
fi

# Use image as kernel if kernel not specified
if [ -z "${KERNEL}" ]; then
    KERNEL="${IMAGE}"
fi

# Build QEMU command
QEMU_CMD=(
    "${QEMU_BIN}"
    "-M" "${MACHINE}"
    "-cpu" "${CPU}"
    "-m" "${MEMORY}"
    "-nographic"
    "-serial" "stdio"
    "-netdev" "user,id=n1,hostfwd=tcp::2222-:22"
    "-device" "${NET_DEV},netdev=n1"
)

# Add kernel
QEMU_CMD+=("-kernel" "${KERNEL}")

# Add kernel arguments
QEMU_CMD+=("-append" "${KERNEL_ARGS}")

# Add DTB if provided
if [ -n "${DTB:-}" ]; then
    QEMU_CMD+=("-dtb" "${DTB}")
fi

# Add rootfs
if [ -n "${ROOTFS}" ]; then
    if [[ "${ROOTFS}" == *.cpio.gz ]]; then
        QEMU_CMD+=("-initrd" "${ROOTFS}")
    elif [[ "${ROOTFS}" == *.ext4 ]]; then
        QEMU_CMD+=("-drive" "file=${ROOTFS},format=raw,if=virtio")
    elif [[ "${ROOTFS}" == *.tar ]]; then
        log_warn "Tar rootfs not directly supported - extract first"
    else
        QEMU_CMD+=("-drive" "file=${ROOTFS},format=raw,if=virtio")
    fi
fi

# Add SD card image if available
SDCARD_IMG="${BUILD_DIR}/mobile-backhaul/buildroot/output/images/sdcard.img"
if [ -f "${SDCARD_IMG}" ]; then
    QEMU_CMD+=("-sd" "${SDCARD_IMG}")
fi

# Start QEMU with timeout
log_info "Starting QEMU test for architecture: ${ARCH}"
log_info "Command: ${QEMU_CMD[*]}"
log_info "Log file: ${LOG_FILE}"
log_info "Timeout: ${TIMEOUT}s"
echo ""

START_TIME=$(date +%s)
QEMU_EXIT_CODE=0

# Run QEMU with timeout
if [ "${VERBOSE}" = true ]; then
    timeout "${TIMEOUT}" "${QEMU_CMD[@]}" 2>&1 | tee "${LOG_FILE}"
    QEMU_EXIT_CODE=$?
else
    timeout "${TIMEOUT}" "${QEMU_CMD[@]}" > "${LOG_FILE}" 2>&1 || QEMU_EXIT_CODE=$?
fi

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

# Analyze results
BOOT_STATUS="unknown"
BOOT_LOG="${TEST_DIR}/boot-$(date +%Y%m%d-%H%M%S).log"
cp "${LOG_FILE}" "${BOOT_LOG}" 2>/dev/null || true

# Check for common boot indicators
if grep -q "Kernel boot successful\|Booting kernel\|Uncompressing Linux\|Starting kernel" "${BOOT_FILE}" 2>/dev/null; then
    BOOT_STATUS="kernel_booted"
fi

if grep -q "init:.*started\|systemd.*started\|init.*started\|Booting completed" "${BOOT_FILE}" 2>/dev/null; then
    BOOT_STATUS="boot_completed"
fi

if grep -q "ttyAMA0\|ttyS0\|console" "${BOOT_FILE}" 2>/dev/null; then
    BOOT_STATUS="${BOOT_STATUS}+console"
fi

# Run verification script if provided
VERIFY_RESULT="skipped"
if [ -n "${VERIFY_SCRIPT}" ] && [ -f "${VERIFY_SCRIPT}" ]; then
    log_info "Running verification script: ${VERIFY_SCRIPT}"
    if bash "${VERIFY_SCRIPT}" "${ARCH}" "${LOG_FILE}" 2>&1 | tee "${TEST_DIR}/verify.log"; then
        VERIFY_RESULT="passed"
    else
        VERIFY_RESULT="failed"
    fi
fi

# Determine test result
TEST_RESULT="failed"
if [ ${QEMU_EXIT_CODE} -eq 0 ] || [ ${QEMU_EXIT_CODE} -eq 124 ]; then
    if [ "${BOOT_STATUS}" = "boot_completed" ] || [ "${BOOT_STATUS}" = "kernel_booted" ]; then
        TEST_RESULT="passed"
    elif [ ${QEMU_EXIT_CODE} -eq 0 ]; then
        TEST_RESULT="passed"
    fi
fi

# Generate test report
REPORT_FILE="${TEST_DIR}/report-$(date +%Y%m%d-%H%M%S).json"
cat > "${REPORT_FILE}" <<EOF
{
  "test": "qemu-boot-test",
  "architecture": "${ARCH}",
  "qemu_binary": "${QEMU_BIN}",
  "machine": "${MACHINE}",
  "cpu": "${CPU}",
  "memory_mb": ${MEMORY},
  "kernel": "${KERNEL}",
  "rootfs": "${ROOTFS:-none}",
  "boot_status": "${BOOT_STATUS}",
  "test_result": "${TEST_RESULT}",
  "verify_result": "${VERIFY_RESULT}",
  "exit_code": ${QEMU_EXIT_CODE},
  "duration_seconds": ${DURATION},
  "timeout": ${TIMEOUT},
  "log_file": "${BOOT_LOG}",
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF

# Print summary
echo ""
echo "========================================================"
echo "  QEMU Boot Test Result"
echo "========================================================"
echo ""
echo -e "  Architecture:    ${ARCH}"
echo -e "  Boot status:     ${BOOT_STATUS}"
echo -e "  Test result:     ${TEST_RESULT}"
echo -e "  Verify result:   ${VERIFY_RESULT}"
echo -e "  Exit code:       ${QEMU_EXIT_CODE}"
echo -e "  Duration:        ${DURATION}s"
echo -e "  Log file:        ${BOOT_LOG}"
echo -e "  Report:          ${REPORT_FILE}"
echo ""

# Exit with appropriate code
if [ "${TEST_RESULT}" = "passed" ]; then
    exit 0
else
    exit 1
fi
