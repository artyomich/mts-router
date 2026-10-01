#!/bin/bash
# test-all.sh — Запуск всех QEMU boot тестов для MTS Router образов
#
# Usage:
#   ./test-all.sh [--arch <arch>] [--clean] [--verbose] [--help]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_RESULTS_DIR="${BUILD_DIR}/test-results/qemu"
LOG_DIR="${TEST_RESULTS_DIR}"

ARCH=""
CLEAN=false
VERBOSE=false

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_success() { echo -e "${GREEN}[PASS]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[SKIP]${NC} $*"; }
log_error() { echo -e "${RED}[FAIL]${NC} $*"; }

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Run all QEMU boot tests for MTS Router images.

Options:
  --arch <arch>        Test specific architecture (aarch64, mips64el, arm)
  --clean              Clean test results before running
  --verbose            Enable verbose output
  --help               Show this help message

Architectures:
  all                  All architectures (default)
  aarch64              MTS-MB-3000, MTS-MC-5000
  mips64el             MTS-OLT-2000
  arm                  MTS-ER-1000, MTS-RG-500

Examples:
  $(basename "$0")
  $(basename "$0") --arch aarch64
  $(basename "$0") --clean --verbose
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --arch)
            ARCH="$2"
            shift 2
            ;;
        --clean)
            CLEAN=true
            shift
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

# Create results directory
mkdir -p "${TEST_RESULTS_DIR}"
RESULTS_FILE="${TEST_RESULTS_DIR}/results.json"

# Clean if requested
if [ "$CLEAN" = true ]; then
    log_info "Cleaning test results..."
    rm -rf "${TEST_RESULTS_DIR}"/*
    log_success "Clean completed"
fi

# Initialize counters
PASS_COUNT=0
FAIL_COUNT=0
SKIP_COUNT=0
TOTAL_COUNT=0

# Test result array
declare -A TEST_RESULTS

# Function to run a single device test
run_device_test() {
    local device_name="$1"
    local device_arch="$2"
    local kernel_path="$3"
    local rootfs_path="$4"
    local dtb_path="${5:-}"

    TOTAL_COUNT=$((TOTAL_COUNT + 1))
    local test_key="${device_name}"

    # Check if image exists
    if [ ! -f "${kernel_path}" ]; then
        log_warn "${device_name}: Kernel not found at ${kernel_path}"
        TEST_RESULTS[${test_key}]="skipped"
        SKIP_COUNT=$((SKIP_COUNT + 1))
        return
    fi

    log_info "Testing ${device_name} (${device_arch})..."

    # Run QEMU test
    local test_args=(
        --arch "${device_arch}"
        --kernel "${kernel_path}"
        --timeout 180
    )

    if [ -n "${rootfs_path}" ] && [ -f "${rootfs_path}" ]; then
        test_args+=(--rootfs "${rootfs_path}")
    fi

    if [ -n "${dtb_path}" ] && [ -f "${dtb_path}" ]; then
        test_args+=(--dtb "${dtb_path}")
    fi

    if bash "${SCRIPT_DIR}/run-test.sh" "${test_args[@]}" 2>&1; then
        log_success "${device_name}: Boot test passed"
        TEST_RESULTS[${test_key}]="passed"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        log_error "${device_name}: Boot test failed"
        TEST_RESULTS[${test_key}]="failed"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

# Generate test report
generate_report() {
    local report_file="${TEST_RESULTS_DIR}/results.json"

    cat > "${report_file}" <<EOF
{
  "test_suite": "qemu-boot-tests",
  "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "summary": {
    "total": ${TOTAL_COUNT},
    "passed": ${PASS_COUNT},
    "failed": ${FAIL_COUNT},
    "skipped": ${SKIP_COUNT}
  },
  "tests": {
EOF

    local first=true
    for key in "${!TEST_RESULTS[@]}"; do
        if [ "$first" = true ]; then
            first=false
        else
            echo "," >> "${report_file}"
        fi
        echo -n "    \"${key}\": \"${TEST_RESULTS[$key]}\"" >> "${report_file}"
    done

    cat >> "${report_file}" <<EOF

  }
}
EOF
}

# Main execution
echo "========================================================"
echo "  MTS Router QEMU Boot Test Suite"
echo "========================================================"
echo ""
echo "Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "Build dir: ${BUILD_DIR}"
echo ""

# Determine which architectures to test
if [ -z "${ARCH}" ] || [ "${ARCH}" = "all" ]; then
    ARCHS=("aarch64" "mips64el" "arm")
else
    ARCHS=("${ARCH}")
fi

# Run tests for each architecture
for arch in "${ARCHS[@]}"; do
    echo "--------------------------------------------------------"
    echo "  Architecture: ${arch}"
    echo "--------------------------------------------------------"

    case "${arch}" in
        aarch64)
            # MTS-MB-3000 (Mobile Backhaul)
            run_device_test "mts-mb3000" "aarch64" \
                "${BUILD_DIR}/mobile-backhaul/buildroot/output/images/uImage" \
                "${BUILD_DIR}/mobile-backhaul/buildroot/output/images/rootfs.cpio.gz"

            # MTS-MC-5000 (Mobile Core)
            run_device_test "mts-mc5000" "aarch64" \
                "${BUILD_DIR}/mobile-core/yocto/tmp/deploy/images/mts-mc5000/Image" \
                "${BUILD_DIR}/mobile-core/yocto/tmp/deploy/images/mts-mc5000/rootfs.ext4"
            ;;

        mips64el)
            # MTS-OLT-2000 (OLT GPON)
            run_device_test "mts-olt2000" "mips64el" \
                "${BUILD_DIR}/olt-gpon/openwrt/bin/targets/*/vmlinux-initrd.bin" \
                ""
            ;;

        arm)
            # MTS-ER-1000 (Enterprise Router)
            run_device_test "mts-er1000" "arm" \
                "${BUILD_DIR}/enterprise-router/openwrt/bin/targets/*/zImage" \
                "${BUILD_DIR}/enterprise-router/openwrt/bin/targets/*/*-rootfs.ext4"

            # MTS-RG-500 (Residential Gateway)
            run_device_test "mts-rg500" "arm" \
                "${BUILD_DIR}/residential-gateway/openwrt/bin/targets/*/zImage" \
                "${BUILD_DIR}/residential-gateway/openwrt/bin/targets/*/*-rootfs.ext4"
            ;;
    esac

    echo ""
done

# Generate final report
generate_report

# Print summary
echo "========================================================"
echo "  QEMU Boot Test Summary"
echo "========================================================"
echo ""
echo -e "  Total tests:   ${TOTAL_COUNT}"
echo -e "  ${GREEN}Passed:        ${PASS_COUNT}${NC}"
echo -e "  ${RED}Failed:        ${FAIL_COUNT}${NC}"
echo -e "  ${YELLOW}Skipped:       ${SKIP_COUNT}${NC}"
echo ""
echo "  Results: ${RESULTS_FILE}"
echo "  Completed: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo ""

# Exit with appropriate code
if [ ${FAIL_COUNT} -gt 0 ]; then
    exit 1
fi
if [ ${SKIP_COUNT} -eq ${TOTAL_COUNT} ]; then
    exit 2
fi
exit 0
