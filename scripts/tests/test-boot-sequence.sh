#!/bin/bash
# MTS Router — Boot Sequence Test Suite
# Tests boot sequence for all MTS Router devices
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
RESULTS_DIR="${BUILD_DIR}/test-results/boot"
LOG_DIR="${RESULTS_DIR}/logs"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PASS=0
FAIL=0
SKIP=0

mkdir -p "${RESULTS_DIR}" "${LOG_DIR}"

# ============================================================
# Logging functions
# ============================================================
log() { echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"; }
log_error() { echo -e "${RED}[FAIL]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }

# ============================================================
# Test result tracking
# ============================================================
record_result() {
    local test_name="$1"
    local status="$2"
    local message="$3"
    local device="${4:-all}"
    
    local result_file="${RESULTS_DIR}/${device}-boot-results.json"
    
    if [ ! -f "${result_file}" ]; then
        echo '{"device": "'${device}'", "tests": [], "timestamp": "'$(date -u +%Y-%m-%dT%H:%M:%SZ)'", "pass": 0, "fail": 0, "skip": 0}' > "${result_file}"
    fi
    
    case "${status}" in
        pass) PASS=$((PASS + 1)) ;;
        fail) FAIL=$((FAIL + 1)) ;;
        skip) SKIP=$((SKIP + 1)) ;;
    esac
    
    log_info "Test [${status^^}]: ${test_name} - ${message}"
}

# ============================================================
# Device list
# ============================================================
declare -A DEVICES
DEVICES=(
    ["mts-cr9000"]="Core Router"
    ["mts-mc5000"]="Mobile Core"
    ["mts-mb3000"]="Mobile Backhaul"
    ["mts-olt2000"]="OLT GPON"
    ["mts-er1000"]="Enterprise Router"
    ["mts-rg500"]="Residential Gateway"
)

# ============================================================
# Test functions
# ============================================================

# Test 1: U-Boot bootloader verification
test_uboot() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local uboot_file="${BUILD_DIR}/${device}/u-boot.bin"
    
    log_info "Testing U-Boot for ${device_name}..."
    
    if [ ! -f "${uboot_file}" ]; then
        log_warn "U-Boot binary not found: ${uboot_file}"
        record_result "uboot-exists" "skip" "U-Boot binary not found" "${device}"
        return 1
    fi
    
    # Check file type
    local file_type=$(file "${uboot_file}" 2>/dev/null || echo "unknown")
    if echo "${file_type}" | grep -qE "ELF|binary|U-Boot"; then
        record_result "uboot-exists" "pass" "U-Boot binary valid: ${file_type}" "${device}"
        return 0
    else
        record_result "uboot-exists" "fail" "Invalid U-Boot binary format: ${file_type}" "${device}"
        return 1
    fi
}

# Test 2: Device tree verification
test_dtb() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local dtb_file="${BUILD_DIR}/${device}/boot/mts-${device}.dtb"
    local dtb_files=()
    
    log_info "Testing Device Tree for ${device_name}..."
    
    # Find DTB files
    if [ -d "${BUILD_DIR}/${device}/boot" ]; then
        dtb_files=($(find "${BUILD_DIR}/${device}/boot" -name "*.dtb" 2>/dev/null || true))
    fi
    
    # Also check deploy directory
    local deploy_dtb="${BUILD_DIR}/deploy-images/${device}"/*.dtb
    if [ -f "${deploy_dtb}" ]; then
        dtb_files+=("${deploy_dtb}")
    fi
    
    if [ ${#dtb_files[@]} -eq 0 ]; then
        log_warn "No DTB files found for ${device}"
        record_result "dtb-exists" "skip" "No DTB files found" "${device}"
        return 1
    fi
    
    local dtb_ok=0
    for dtb in "${dtb_files[@]}"; do
        if [ -f "${dtb}" ]; then
            local file_type=$(file "${dtb}" 2>/dev/null || echo "unknown")
            if echo "${file_type}" | grep -q "device tree"; then
                record_result "dtb-valid" "pass" "DTB valid: $(basename ${dtb})" "${device}"
                dtb_ok=1
            fi
        fi
    done
    
    if [ ${dtb_ok} -eq 0 ]; then
        record_result "dtb-valid" "fail" "No valid DTB files found" "${device}"
        return 1
    fi
    
    return 0
}

# Test 3: Kernel image verification
test_kernel() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local kernel_file="${BUILD_DIR}/${device}/boot/vmlinuz"
    
    log_info "Testing Kernel for ${device_name}..."
    
    if [ ! -f "${kernel_file}" ]; then
        # Try alternate locations
        kernel_file="${BUILD_DIR}/deploy-images/${device}/vmlinuz"
        kernel_file=$(find "${BUILD_DIR}" -name "vmlinuz*" -o -name "zImage*" -o -name "Image" 2>/dev/null | head -1)
    fi
    
    if [ -z "${kernel_file}" ] || [ ! -f "${kernel_file}" ]; then
        log_warn "Kernel image not found for ${device}"
        record_result "kernel-exists" "skip" "Kernel image not found" "${device}"
        return 1
    fi
    
    # Check kernel magic number (ELF or U-Boot ARM image)
    local magic=$(xxd -l 4 -p "${kernel_file}" 2>/dev/null || echo "")
    
    if [ -n "${magic}" ]; then
        # ELF magic: 7f454c46
        # ARM kernel magic: 0161a1de (for older kernels) or 6400a8a0 (for newer)
        if [[ "${magic}" == "7f454c46"* ]] || [[ "${magic}" == "0161a1de" ]] || [[ "${magic}" == "6400a8a0"* ]]; then
            record_result "kernel-valid" "pass" "Kernel magic valid: ${magic}" "${device}"
            return 0
        else
            # Could be compressed kernel - check further
            local file_type=$(file "${kernel_file}" 2>/dev/null || echo "unknown")
            if echo "${file_type}" | grep -qiE "kernel|ELF|gzip|compressed"; then
                record_result "kernel-valid" "pass" "Kernel appears valid: ${file_type}" "${device}"
                return 0
            fi
        fi
    fi
    
    record_result "kernel-valid" "fail" "Invalid kernel image" "${device}"
    return 1
}

# Test 4: Initramfs/initrd verification
test_initrd() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local initrd_file="${BUILD_DIR}/${device}/boot/initrd.img"
    
    log_info "Testing InitRD for ${device_name}..."
    
    if [ ! -f "${initrd_file}" ]; then
        # Try alternate locations
        initrd_file=$(find "${BUILD_DIR}" -name "initrd*" -type f 2>/dev/null | head -1)
    fi
    
    if [ -z "${initrd_file}" ] || [ ! -f "${initrd_file}" ]; then
        log_warn "InitRD not found for ${device} (may not be needed)"
        record_result "initrd-exists" "skip" "InitRD not found (may be optional)" "${device}"
        return 0
    fi
    
    # Check initrd format
    local file_type=$(file "${initrd_file}" 2>/dev/null || echo "unknown")
    if echo "${file_type}" | grep -qiE "gzip|cpio|ext|filesystem|archive"; then
        record_result "initrd-valid" "pass" "InitRD format valid: ${file_type}" "${device}"
        return 0
    else
        record_result "initrd-valid" "fail" "Invalid InitRD format: ${file_type}" "${device}"
        return 1
    fi
}

# Test 5: Root filesystem verification
test_rootfs() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local rootfs_files=()
    
    log_info "Testing RootFS for ${device_name}..."
    
    # Find rootfs images
    rootfs_files=$(find "${BUILD_DIR}" -path "*/images/*" -name "*.ext4" -o -path "*/images/*" -name "*.squashfs" -o -path "*/images/*" -name "*.rootfs" 2>/dev/null | head -10)
    
    if [ -z "${rootfs_files}" ]; then
        log_warn "No rootfs images found for ${device}"
        record_result "rootfs-exists" "skip" "No rootfs images found" "${device}"
        return 1
    fi
    
    local rootfs_ok=0
    while IFS= read -r rootfs; do
        if [ -f "${rootfs}" ]; then
            local file_type=$(file "${rootfs}" 2>/dev/null || echo "unknown")
            if echo "${file_type}" | grep -qiE "ext[234]|squashfs|filesystem|data"; then
                local size=$(stat -c%s "${rootfs}" 2>/dev/null || stat -f%z "${rootfs}" 2>/dev/null || echo "unknown")
                record_result "rootfs-valid" "pass" "RootFS valid: $(basename ${rootfs}) (${size} bytes)" "${device}"
                rootfs_ok=1
            fi
        fi
    done <<< "${rootfs_files}"
    
    if [ ${rootfs_ok} -eq 0 ]; then
        record_result "rootfs-valid" "fail" "No valid rootfs images found" "${device}"
        return 1
    fi
    
    return 0
}

# Test 6: Boot config verification
test_boot_config() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local extlinux_conf="${BUILD_DIR}/${device}/boot/extlinux/extlinux.conf"
    
    log_info "Testing Boot Config for ${device_name}..."
    
    if [ ! -f "${extlinux_conf}" ]; then
        # Try alternate locations
        extlinux_conf=$(find "${BUILD_DIR}" -name "extlinux.conf" 2>/dev/null | head -1)
    fi
    
    if [ -z "${extlinux_conf}" ] || [ ! -f "${extlinux_conf}" ]; then
        log_warn "Boot config not found for ${device}"
        record_result "boot-config" "skip" "Boot config not found" "${device}"
        return 1
    fi
    
    # Check for required boot parameters
    local has_kernel=0
    local has_root=0
    local has_console=0
    
    if grep -q "KERNEL" "${extlinux_conf}"; then has_kernel=1; fi
    if grep -q "APPEND" "${extlinux_conf}"; then has_root=1; fi
    if grep -q "console" "${extlinux_conf}"; then has_console=1; fi
    
    local result="pass"
    local message="Boot config valid"
    
    if [ ${has_kernel} -eq 0 ]; then
        result="fail"
        message="${message} missing KERNEL"
    fi
    if [ ${has_root} -eq 0 ]; then
        result="fail"
        message="${message} missing APPEND"
    fi
    if [ ${has_console} -eq 0 ]; then
        result="warn"
        message="${message} missing console"
    fi
    
    record_result "boot-config" "${result}" "${message}" "${device}"
    
    if [ "${result}" = "fail" ]; then
        return 1
    fi
    return 0
}

# Test 7: Boot script verification
test_boot_script() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local boot_script="${BUILD_DIR}/${device}/boot/boot.scr"
    
    log_info "Testing Boot Script for ${device_name}..."
    
    if [ ! -f "${boot_script}" ]; then
        # Try to find it
        boot_script=$(find "${BUILD_DIR}" -name "boot.scr" 2>/dev/null | head -1)
    fi
    
    if [ -z "${boot_script}" ] || [ ! -f "${boot_script}" ]; then
        log_warn "Boot script not found for ${device} (optional)"
        record_result "boot-script" "skip" "Boot script not found (optional)" "${device}"
        return 0
    fi
    
    # Verify boot script is valid U-Boot script
    if file "${boot_script}" 2>/dev/null | grep -qi "script"; then
        record_result "boot-script" "pass" "Boot script valid" "${device}"
        return 0
    else
        record_result "boot-script" "fail" "Invalid boot script format" "${device}"
        return 1
    fi
}

# Test 8: Firmware files verification
test_firmware() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local firmware_dirs=()
    
    log_info "Testing Firmware for ${device_name}..."
    
    # Look for firmware directories
    if [ -d "${BUILD_DIR}/${device}/etc/firmware" ]; then
        firmware_dirs+=("${BUILD_DIR}/${device}/etc/firmware")
    fi
    if [ -d "${BUILD_DIR}/${device}/lib/firmware" ]; then
        firmware_dirs+=("${BUILD_DIR}/${device}/lib/firmware")
    fi
    if [ -d "${BUILD_DIR}/deploy-images/${device}/firmware" ]; then
        firmware_dirs+=("${BUILD_DIR}/deploy-images/${device}/firmware")
    fi
    
    local firmware_ok=0
    for fw_dir in "${firmware_dirs[@]}"; do
        if [ -d "${fw_dir}" ]; then
            local fw_count=$(find "${fw_dir}" -type f 2>/dev/null | wc -l)
            if [ ${fw_count} -gt 0 ]; then
                record_result "firmware-files" "pass" "${fw_count} firmware files in $(basename ${fw_dir})" "${device}"
                firmware_ok=1
            fi
        fi
    done
    
    if [ ${firmware_ok} -eq 0 ]; then
        record_result "firmware-files" "skip" "No firmware files found (may be loaded at runtime)" "${device}"
    fi
    
    return 0
}

# Test 9: Image checksum verification
test_image_checksum() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    local checksum_files=()
    
    log_info "Testing Image Checksums for ${device_name}..."
    
    # Find checksum files
    checksum_files=$(find "${BUILD_DIR}" -name "*.sha256" -o -name "*.sha512" -o -name "*.md5" 2>/dev/null | grep "${device}" | head -10)
    
    if [ -z "${checksum_files}" ]; then
        log_warn "No checksum files found for ${device}"
        record_result "image-checksum" "skip" "No checksum files found" "${device}"
        return 1
    fi
    
    local checksum_ok=0
    while IFS= read -r checksum_file; do
        if [ -f "${checksum_file}" ]; then
            record_result "checksum-exists" "pass" "Checksum file: $(basename ${checksum_file})" "${device}"
            checksum_ok=1
        fi
    done <<< "${checksum_files}"
    
    if [ ${checksum_ok} -eq 0 ]; then
        record_result "image-checksum" "fail" "No valid checksum files" "${device}"
        return 1
    fi
    
    return 0
}

# Test 10: Boot time measurement (if QEMU available)
test_boot_time() {
    local device="$1"
    local device_name="${DEVICES[$device]:-$device}"
    
    log_info "Testing Boot Time for ${device_name}..."
    
    if ! command -v qemu-system-aarch64 &>/dev/null && ! command -v qemu-system-x86_64 &>/dev/null; then
        log_warn "QEMU not available, skipping boot time test"
        record_result "boot-time" "skip" "QEMU not available" "${device}"
        return 0
    fi
    
    # Find kernel and initrd
    local kernel=$(find "${BUILD_DIR}" -name "vmlinuz*" -o -name "zImage*" 2>/dev/null | grep "${device}" | head -1)
    local initrd=$(find "${BUILD_DIR}" -name "initrd*" 2>/dev/null | grep "${device}" | head -1)
    
    if [ -z "${kernel}" ]; then
        record_result "boot-time" "skip" "No kernel image for boot test" "${device}"
        return 0
    fi
    
    # Boot test with timeout
    local qemu_opts=""
    if command -v qemu-system-aarch64 &>/dev/null; then
        qemu_opts="-M virt -cpu cortex-a53 -nographic -serial null -monitor null"
    elif command -v qemu-system-x86_64 &>/dev/null; then
        qemu_opts="-machine q35 -cpu host -nographic -serial null -monitor null"
    fi
    
    local start_time=$(date +%s%N)
    
    # Run QEMU with timeout (5 minutes max for boot test)
    if timeout 300 qemu-system-aarch64 ${qemu_opts} -kernel "${kernel}" ${initrd:+-initrd ${initrd}} 2>&1 | tee "${LOG_DIR}/${device}-boot-time.log" | grep -q "Booting kernel\|Linux version\|Booted"; then
        local end_time=$(date +%s%N)
        local boot_time=$(( (end_time - start_time) / 1000000 ))
        record_result "boot-time" "pass" "Boot time: ${boot_time}ms" "${device}"
        return 0
    else
        local end_time=$(date +%s%N)
        local boot_time=$(( (end_time - start_time) / 1000000 ))
        log_warn "Boot test timed out or failed after ${boot_time}ms"
        record_result "boot-time" "fail" "Boot test failed after ${boot_time}ms" "${device}"
        return 1
    fi
}

# ============================================================
# Main test execution
# ============================================================
main() {
    log "========================================"
    log "MTS Router Boot Sequence Test Suite"
    log "========================================"
    log "Start time: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    log ""
    
    local devices="${1:-all}"
    
    if [ "${devices}" = "all" ]; then
        for device in "${!DEVICES[@]}"; do
            log ""
            log "========================================"
            log "Testing ${DEVICES[$device]} (${device})"
            log "========================================"
            
            test_uboot "${device}" || true
            test_dtb "${device}" || true
            test_kernel "${device}" || true
            test_initrd "${device}" || true
            test_rootfs "${device}" || true
            test_boot_config "${device}" || true
            test_boot_script "${device}" || true
            test_firmware "${device}" || true
            test_image_checksum "${device}" || true
            test_boot_time "${device}" || true
        done
    else
        log "Testing device: ${devices}"
        test_uboot "${devices}" || true
        test_dtb "${devices}" || true
        test_kernel "${devices}" || true
        test_initrd "${devices}" || true
        test_rootfs "${devices}" || true
        test_boot_config "${devices}" || true
        test_boot_script "${devices}" || true
        test_firmware "${devices}" || true
        test_image_checksum "${devices}" || true
        test_boot_time "${devices}" || true
    fi
    
    # Print summary
    log ""
    log "========================================"
    log "Boot Sequence Test Summary"
    log "========================================"
    log "Total tests: $((PASS + FAIL + SKIP))"
    log -e "Passed: ${GREEN}${PASS}${NC}"
    log -e "Failed: ${RED}${FAIL}${NC}"
    log -e "Skipped: ${YELLOW}${SKIP}${NC}"
    log ""
    
    # Generate JSON report
    cat > "${RESULTS_DIR}/boot-sequence-report.json" << EOF
{
    "test_suite": "boot-sequence",
    "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "summary": {
        "total": $((PASS + FAIL + SKIP)),
        "pass": ${PASS},
        "fail": ${FAIL},
        "skip": ${SKIP}
    },
    "results_dir": "${RESULTS_DIR}",
    "log_dir": "${LOG_DIR}"
}
EOF
    
    if [ ${FAIL} -gt 0 ]; then
        log_error "${FAIL} boot sequence test(s) failed"
        return 1
    fi
    
    log "Boot sequence tests completed successfully"
    return 0
}

# Run main
main "${1:-all}"
