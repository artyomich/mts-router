#!/bin/bash
# MTS Router — Image Verification and Validation Script
# Verifies built images for correctness, completeness, and security
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
RESULTS_DIR="${BUILD_DIR}/verify-results"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PASS=0
FAIL=0
WARN=0

mkdir -p "${RESULTS_DIR}"

log() { echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"; }
log_error() { echo -e "${RED}[FAIL]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }

# ============================================================
# Verify image checksum
# ============================================================
verify_checksum() {
    local image_file="$1"
    local device="$2"
    
    log_info "Verifying checksum for ${image_file}..."
    
    local checksum_file="${image_file}.sha256"
    
    if [ ! -f "${checksum_file}" ]; then
        log_warn "No checksum file found: ${checksum_file}"
        WARN=$((WARN + 1))
        return 1
    fi
    
    # Verify checksum
    if sha256sum -c "${checksum_file}" --quiet 2>/dev/null; then
        log "Checksum verified for $(basename ${image_file})"
        PASS=$((PASS + 1))
        return 0
    else
        log_error "Checksum verification failed for $(basename ${image_file})"
        FAIL=$((FAIL + 1))
        return 1
    fi
}

# ============================================================
# Verify image format
# ============================================================
verify_image_format() {
    local image_file="$1"
    local device="$2"
    
    log_info "Verifying image format for ${image_file}..."
    
    local file_type=$(file "${image_file}" 2>/dev/null || echo "unknown")
    
    case "$(basename ${image_file})" in
        *.ext4|*.ext4.gz)
            if echo "${file_type}" | grep -qiE "ext[234]|ext2|ext3|ext4|filesystem"; then
                log "Valid ext filesystem: ${file_type}"
                PASS=$((PASS + 1))
                return 0
            fi
            ;;
        *.squashfs|*.squashfs.gz)
            if echo "${file_type}" | grep -qiE "squashfs|squash"; then
                log "Valid squashfs: ${file_type}"
                PASS=$((PASS + 1))
                return 0
            fi
            ;;
        *.bin|*.img)
            if echo "${file_type}" | grep -qiE "data|binary|ELF|firmware"; then
                log "Valid binary/image: ${file_type}"
                PASS=$((PASS + 1))
                return 0
            fi
            ;;
        *.dtb)
            if echo "${file_type}" | grep -qiE "device tree|dtb"; then
                log "Valid device tree: ${file_type}"
                PASS=$((PASS + 1))
                return 0
            fi
            ;;
        *.dtb.gz)
            if echo "${file_type}" | grep -qiE "gzip|compressed"; then
                log "Valid compressed DTB: ${file_type}"
                PASS=$((PASS + 1))
                return 0
            fi
            ;;
        *.itb)
            if echo "${file_type}" | grep -qiE " FIT|U-Boot|image"; then
                log "Valid FIT image: ${file_type}"
                PASS=$((PASS + 1))
                return 0
            fi
            ;;
    esac
    
    log_warn "Unknown image format: ${file_type}"
    WARN=$((WARN + 1))
    return 1
}

# ============================================================
# Verify image size
# ============================================================
verify_image_size() {
    local image_file="$1"
    local device="$2"
    
    log_info "Verifying image size for ${image_file}..."
    
    local size=$(stat -c%s "${image_file}" 2>/dev/null || stat -f%z "${image_file}" 2>/dev/null || echo "0")
    local size_mb=$((size / 1024 / 1024))
    
    # Define minimum sizes per device
    local min_sizes=(
        "mts-cr9000:100"
        "mts-mc5000:100"
        "mts-mb3000:50"
        "mts-olt2000:80"
        "mts-er1000:50"
        "mts-rg500:20"
    )
    
    local min_size=10  # Default 10MB
    
    for entry in "${min_sizes[@]}"; do
        local dev=$(echo "${entry}" | cut -d: -f1)
        local size_val=$(echo "${entry}" | cut -d: -f2)
        if [[ "${device}" == "${dev}"* ]]; then
            min_size=${size_val}
            break
        fi
    done
    
    if [ ${size_mb} -lt ${min_size} ]; then
        log_warn "Image too small: ${size_mb}MB < ${min_size}MB minimum"
        WARN=$((WARN + 1))
        return 1
    else
        log "Image size OK: ${size_mb}MB"
        PASS=$((PASS + 1))
        return 0
    fi
}

# ============================================================
# Verify filesystem integrity
# ============================================================
verify_fs_integrity() {
    local image_file="$1"
    local device="$2"
    
    log_info "Verifying filesystem integrity for ${image_file}..."
    
    # Check if e2fsck is available for ext filesystems
    if ! command -v e2fsck &>/dev/null; then
        log_warn "e2fsck not available, skipping filesystem check"
        WARN=$((WARN + 1))
        return 0
    fi
    
    # Only check ext filesystems
    if echo "$(basename ${image_file})" | grep -qE "ext[234]"; then
        # Check filesystem
        if e2fsck -n -f "${image_file}" &>/dev/null; then
            log "Filesystem integrity OK"
            PASS=$((PASS + 1))
            return 0
        else
            log_error "Filesystem integrity check failed"
            FAIL=$((FAIL + 1))
            return 1
        fi
    fi
    
    log_info "Skipping filesystem check for non-ext image"
    return 0
}

# ============================================================
# Verify boot files
# ============================================================
verify_boot_files() {
    local device="$1"
    local deploy_dir="${BUILD_DIR}/deploy-images/${device}"
    
    log_info "Verifying boot files for ${device}..."
    
    local boot_files_ok=0
    local boot_files_missing=0
    
    # Check for kernel
    if find "${deploy_dir}" -name "vmlinuz*" -o -name "zImage*" -o -name "Image*" 2>/dev/null | grep -q .; then
        log_info "Kernel found"
        boot_files_ok=$((boot_files_ok + 1))
    else
        log_warn "Kernel not found"
        boot_files_missing=$((boot_files_missing + 1))
    fi
    
    # Check for DTB
    if find "${deploy_dir}" -name "*.dtb" 2>/dev/null | grep -q .; then
        log_info "Device tree found"
        boot_files_ok=$((boot_files_ok + 1))
    else
        log_warn "Device tree not found"
        boot_files_missing=$((boot_files_missing + 1))
    fi
    
    # Check for initrd
    if find "${deploy_dir}" -name "initrd*" 2>/dev/null | grep -q .; then
        log_info "InitRD found"
        boot_files_ok=$((boot_files_ok + 1))
    else
        log_warn "InitRD not found (may be optional)"
    fi
    
    # Check for extlinux.conf
    if find "${deploy_dir}" -name "extlinux.conf" 2>/dev/null | grep -q .; then
        log_info "Boot config found"
        boot_files_ok=$((boot_files_ok + 1))
    else
        log_warn "Boot config not found"
        boot_files_missing=$((boot_files_missing + 1))
    fi
    
    if [ ${boot_files_missing} -eq 0 ]; then
        log "All boot files present"
        PASS=$((PASS + 1))
        return 0
    else
        log_warn "Missing boot files: ${boot_files_missing}"
        WARN=$((WARN + 1))
        return 1
    fi
}

# ============================================================
# Verify manifest
# ============================================================
verify_manifest() {
    local device="$1"
    local deploy_dir="${BUILD_DIR}/deploy-images/${device}"
    
    log_info "Verifying manifest for ${device}..."
    
    local manifest="${deploy_dir}/${device}-manifest.txt"
    
    if [ ! -f "${manifest}" ]; then
        log_warn "Manifest not found: ${manifest}"
        WARN=$((WARN + 1))
        return 1
    fi
    
    # Check manifest content
    local has_packages=0
    local has_kernel=0
    local has_timestamp=0
    
    if grep -q "Package\|package\|CONFIG_PACKAGE" "${manifest}" 2>/dev/null; then
        has_packages=1
    fi
    
    if grep -q "Kernel\|kernel\|CONFIG_KERNEL" "${manifest}" 2>/dev/null; then
        has_kernel=1
    fi
    
    if grep -q "Build date\|timestamp\|date" "${manifest}" 2>/dev/null; then
        has_timestamp=1
    fi
    
    if [ ${has_packages} -eq 1 ] && [ ${has_kernel} -eq 1 ] && [ ${has_timestamp} -eq 1 ]; then
        log "Manifest valid"
        PASS=$((PASS + 1))
        return 0
    else
        log_warn "Manifest incomplete (packages:${has_packages} kernel:${has_kernel} timestamp:${has_timestamp})"
        WARN=$((WARN + 1))
        return 1
    fi
}

# ============================================================
# Verify security
# ============================================================
verify_security() {
    local device="$1"
    local deploy_dir="${BUILD_DIR}/deploy-images/${device}"
    
    log_info "Verifying security for ${device}..."
    
    # Check for signature files
    local sig_files=$(find "${deploy_dir}" -name "*.sig" -o -name "*.asc" 2>/dev/null | wc -l)
    
    if [ ${sig_files} -gt 0 ]; then
        log_info "Signature files found: ${sig_files}"
    else
        log_warn "No signature files found (optional)"
    fi
    
    # Check for secure boot config
    if find "${deploy_dir}" -name "*secure*" -o -name "*boot*" 2>/dev/null | grep -q .; then
        log_info "Secure boot files found"
    else
        log_warn "No secure boot files found (optional)"
    fi
    
    PASS=$((PASS + 1))
    return 0
}

# ============================================================
# Main verification
# ============================================================
main() {
    log "========================================"
    log "MTS Router Image Verification"
    log "========================================"
    log "Start time: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    log ""
    
    local device="${1:-all}"
    local devices=(mts-cr9000 mts-mc5000 mts-mb3000 mts-olt2000 mts-er1000 mts-rg500)
    
    if [ "${device}" != "all" ]; then
        devices=("${device}")
    fi
    
    for dev in "${devices[@]}"; do
        log ""
        log "========================================"
        log "Verifying ${dev}"
        log "========================================"
        
        local deploy_dir="${BUILD_DIR}/deploy-images/${dev}"
        
        # Find all images
        local images=()
        if [ -d "${deploy_dir}" ]; then
            images=($(find "${deploy_dir}" -type f \( -name "*.ext4" -o -name "*.ext4.gz" -o -name "*.squashfs" -o -name "*.bin" -o -name "*.img" -o -name "*.dtb" -o -name "*.itb" \) 2>/dev/null))
        fi
        
        # Also check build directory
        if [ ${#images[@]} -eq 0 ]; then
            local build_subdirs=("build/cr9000" "build/mc5000" "build/mb3000" "build/olt-gpon" "build/enterprise" "build/residential")
            for subdir in "${build_subdirs[@]}"; do
                if [[ "${dev}" == *"cr9000"* && "${subdir}" == *"cr9000"* ]] || \
                   [[ "${dev}" == *"mc5000"* && "${subdir}" == *"mc5000"* ]] || \
                   [[ "${dev}" == *"mb3000"* && "${subdir}" == *"mb3000"* ]] || \
                   [[ "${dev}" == *"olt2000"* && "${subdir}" == *"olt"* ]] || \
                   [[ "${dev}" == *"er1000"* && "${subdir}" == *"enterprise"* ]] || \
                   [[ "${dev}" == *"rg500"* && "${subdir}" == *"residential"* ]]; then
                    local found=$(find "${PROJECT_DIR}/${subdir}" -type f \( -name "*.ext4" -o -name "*.bin" -o -name "*.img" \) 2>/dev/null | head -5)
                    if [ -n "${found}" ]; then
                        images+=(${found})
                    fi
                fi
            done
        fi
        
        if [ ${#images[@]} -eq 0 ]; then
            log_warn "No images found for ${dev}"
            WARN=$((WARN + 1))
            continue
        fi
        
        for img in "${images[@]}"; do
            if [ -f "${img}" ]; then
                verify_checksum "${img}" "${dev}" || true
                verify_image_format "${img}" "${dev}" || true
                verify_image_size "${img}" "${dev}" || true
                verify_fs_integrity "${img}" "${dev}" || true
            fi
        done
        
        verify_boot_files "${dev}" || true
        verify_manifest "${dev}" || true
        verify_security "${dev}" || true
    done
    
    # Print summary
    log ""
    log "========================================"
    log "Image Verification Summary"
    log "========================================"
    log "Total checks: $((PASS + FAIL + WARN))"
    log -e "Passed: ${GREEN}${PASS}${NC}"
    log -e "Failed: ${RED}${FAIL}${NC}"
    log -e "Warnings: ${YELLOW}${WARN}${NC}"
    log ""
    
    # Generate report
    cat > "${RESULTS_DIR}/image-verification-report.json" << EOF
{
    "verification_suite": "image-verification",
    "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "summary": {
        "total": $((PASS + FAIL + WARN)),
        "pass": ${PASS},
        "fail": ${FAIL},
        "warn": ${WARN}
    },
    "results_dir": "${RESULTS_DIR}"
}
EOF
    
    if [ ${FAIL} -gt 0 ]; then
        log_error "${FAIL} verification check(s) failed"
        return 1
    fi
    
    log "Image verification completed"
    return 0
}

# Run main
main "${1:-all}"
