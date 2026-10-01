#!/bin/bash
# Yocto/Buildroot/OpenWrt Build Orchestration Script with Artifact Validation
#
# This script orchestrates building OS images for all MTS Router devices
# using their respective build systems:
#   - MTS-CR-9000, MTS-MC-5000: Yocto (meta-mts layer)
#   - MTS-MB-3000: Buildroot
#   - MTS-OLT-2000, MTS-ER-1000, MTS-RG-500: OpenWrt
#
# Usage:
#   ./build-yocto-image.sh [--device <device>] [--clean] [--validate] [--help]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Default settings
DEVICE="all"
CLEAN=false
VALIDATE=false
BUILD_DIR="${PROJECT_DIR}/build"
BUILD_LOGS_DIR="${BUILD_DIR}/logs"
RESULTS_FILE="${BUILD_DIR}/build-results.json"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

mkdir -p "${BUILD_LOGS_DIR}"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --device|-d)
            DEVICE="$2"
            shift 2
            ;;
        --clean|-c)
            CLEAN=true
            shift
            ;;
        --validate|-v)
            VALIDATE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--device <device>] [--clean] [--validate] [--help]"
            echo ""
            echo "Build OS images for MTS Router devices"
            echo ""
            echo "Devices:"
            echo "  all           - Build all devices (default)"
            echo "  cr9000        - MTS-CR-9000 Core Router (Yocto)"
            echo "  mc5000        - MTS-MC-5000 Mobile Core (Yocto + K3s)"
            echo "  mb3000        - MTS-MB-3000 Mobile Backhaul (Buildroot)"
            echo "  olt2000       - MTS-OLT-2000 OLT GPON (OpenWrt)"
            echo "  er1000        - MTS-ER-1000 Enterprise Router (OpenWrt)"
            echo "  rg500         - MTS-RG-500 Residential Gateway (OpenWrt)"
            echo ""
            echo "Options:"
            echo "  --clean       Clean build artifacts before building"
            echo "  --validate    Validate build artifacts after building"
            echo "  --help        Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

# Logging functions
log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }
log_section() {
    echo ""
    echo -e "${CYAN}========================================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================================${NC}"
}

# Validate Yocto image artifacts
validate_yocto_image() {
    local device="$1"
    local build_path="$2"
    local deploy_dir="${build_path}/tmp/deploy/images/mts-${device,,}"
    local status=0

    log_section "Validating Yocto Image for MTS-${device^^}"

    if [[ ! -d "${deploy_dir}" ]]; then
        log_error "Deploy directory not found: ${deploy_dir}"
        return 1
    fi

    local checks_passed=0
    local checks_total=0

    # Check for kernel image
    checks_total=$((checks_total + 1))
    if ls "${deploy_dir}"/zImage 2>/dev/null || ls "${deploy_dir}"/Image 2>/dev/null; then
        log_success "Kernel image found"
        checks_passed=$((checks_passed + 1))
    else
        log_error "Kernel image (zImage/Image) not found in ${deploy_dir}"
        status=1
    fi

    # Check for device tree
    checks_total=$((checks_total + 1))
    if ls "${deploy_dir}"/*.dtb 2>/dev/null; then
        log_success "Device tree blob found"
        checks_passed=$((checks_passed + 1))
    else
        log_error "Device tree blob (*.dtb) not found in ${deploy_dir}"
        status=1
    fi

    # Check for rootfs
    checks_total=$((checks_total + 1))
    if ls "${deploy_dir}"/*.ext4 2>/dev/null || ls "${deploy_dir}"/*.wic 2>/dev/null; then
        log_success "Root filesystem found"
        checks_passed=$((checks_passed + 1))
    else
        log_error "Root filesystem (*.ext4, *.wic) not found in ${deploy_dir}"
        status=1
    fi

    # Check for wic image
    checks_total=$((checks_total + 1))
    if ls "${deploy_dir}"/*.wic 2>/dev/null; then
        log_success "WIC image found"
        checks_passed=$((checks_passed + 1))
    else
        log_warn "WIC image not found (optional)"
    fi

    # Report validation result
    echo ""
    log_info "Validation result: ${checks_passed}/${checks_total} checks passed"
    return ${status}
}

# Validate Buildroot image artifacts
validate_buildroot_image() {
    local device="$1"
    local build_path="$2"
    local output_dir="${build_path}/output/images"
    local status=0

    log_section "Validating Buildroot Image for MTS-${device^^}"

    if [[ ! -d "${output_dir}" ]]; then
        log_error "Buildroot output directory not found: ${output_dir}"
        return 1
    fi

    local checks_passed=0
    local checks_total=0

    # Check for kernel image
    checks_total=$((checks_total + 1))
    if ls "${output_dir}"/uImage 2>/dev/null || ls "${output_dir}"/zImage 2>/dev/null; then
        log_success "Kernel image found"
        checks_passed=$((checks_passed + 1))
    else
        log_error "Kernel image (uImage/zImage) not found in ${output_dir}"
        status=1
    fi

    # Check for rootfs
    checks_total=$((checks_total + 1))
    if ls "${output_dir}"/rootfs.cpio.gz 2>/dev/null || ls "${output_dir}"/rootfs.tar 2>/dev/null || ls "${output_dir}"/rootfs.ext4 2>/dev/null; then
        log_success "Root filesystem found"
        checks_passed=$((checks_passed + 1))
    else
        log_error "Root filesystem not found in ${output_dir}"
        status=1
    fi

    # Check for SD card image
    checks_total=$((checks_total + 1))
    if ls "${output_dir}"/sdcard.img 2>/dev/null; then
        log_success "SD card image found"
        checks_passed=$((checks_passed + 1))
    else
        log_warn "SD card image not found (optional)"
    fi

    echo ""
    log_info "Validation result: ${checks_passed}/${checks_total} checks passed"
    return ${status}
}

# Validate OpenWrt image artifacts
validate_openwrt_image() {
    local device="$1"
    local build_path="$2"
    local bin_dir="${build_path}/bin/targets"
    local status=0

    log_section "Validating OpenWrt Image for MTS-${device^^}"

    if [[ ! -d "${bin_dir}" ]]; then
        log_error "OpenWrt bin directory not found: ${bin_dir}"
        return 1
    fi

    local checks_passed=0
    local checks_total=0

    # Check for sysupgrade image
    checks_total=$((checks_total + 1))
    if find "${bin_dir}" -name "*-sysupgrade.bin" 2>/dev/null | head -1 | grep -q .; then
        log_success "Sysupgrade image found"
        checks_passed=$((checks_passed + 1))
    else
        log_error "Sysupgrade image not found in ${bin_dir}"
        status=1
    fi

    # Check for rootfs tar
    checks_total=$((checks_total + 1))
    if find "${bin_dir}" -name "*-rootfs.tar.gz" 2>/dev/null | head -1 | grep -q .; then
        log_success "Rootfs tar found"
        checks_passed=$((checks_passed + 1))
    else
        log_warn "Rootfs tar not found (optional)"
    fi

    # Check for factory image
    checks_total=$((checks_total + 1))
    if find "${bin_dir}" -name "*-factory.bin" 2>/dev/null | head -1 | grep -q .; then
        log_success "Factory image found"
        checks_passed=$((checks_passed + 1))
    else
        log_warn "Factory image not found (optional)"
    fi

    # Check for combined image
    checks_total=$((checks_total + 1))
    if find "${bin_dir}" -name "*-combined.img*" 2>/dev/null | head -1 | grep -q .; then
        log_success "Combined image found"
        checks_passed=$((checks_passed + 1))
    else
        log_warn "Combined image not found (optional)"
    fi

    echo ""
    log_info "Validation result: ${checks_passed}/${checks_total} checks passed"
    return ${status}
}

# Build Yocto image for Core Router
build_yocto_core_router() {
    log_section "Building Yocto Image for MTS-CR-9000 (Core Router)"

    local build_path="${BUILD_DIR}/core-router/yocto"
    mkdir -p "${build_path}"
    local log_file="${BUILD_LOGS_DIR}/build-cr9000.log"

    # Check prerequisites
    if ! command -v bitbake &>/dev/null; then
        log_warn "bitbake not found - cannot build Yocto image"
        log_info "Initialize environment: source ${BUILD_DIR}/yocto-work/setup-env.sh"
        return 1
    fi

    # Check meta-mts layer
    if [[ ! -d "${PROJECT_DIR}/linux/meta-mts" ]]; then
        log_error "meta-mts layer not found at ${PROJECT_DIR}/linux/meta-mts"
        return 1
    fi

    # Source Yocto environment
    log_info "Sourcing Yocto environment..."
    local poky_dir="${BUILD_DIR}/yocto-work/poky"
    if [[ -d "${poky_dir}" ]]; then
        source "${poky_dir}/oe-init-build-env" "${build_path}" 2>/dev/null || true
    else
        log_warn "Poky not found at ${poky_dir} - run init-yocto-env.sh first"
        return 1
    fi

    # Add layers
    bitbake-layers add-layer ../../linux/meta-openembedded/meta-oe 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-openembedded/meta-networking 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-openembedded/meta-python 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-mts 2>/dev/null || true

    # Build
    log_info "Running: bitbake mts-core-router-image"
    if bitbake mts-core-router-image 2>&1 | tee "${log_file}"; then
        log_success "Yocto build for CR-9000 completed"
        if [[ "${VALIDATE}" = true ]]; then
            validate_yocto_image "CR9000" "${build_path}"
        fi
        return 0
    else
        log_error "Yocto build for CR-9000 failed - see ${log_file}"
        return 1
    fi
}

# Build Yocto image for Mobile Core
build_yocto_mobile_core() {
    log_section "Building Yocto + K3s Image for MTS-MC-5000 (Mobile Core)"

    local build_path="${BUILD_DIR}/mobile-core/yocto"
    mkdir -p "${build_path}"
    local log_file="${BUILD_LOGS_DIR}/build-mc5000.log"

    # Check prerequisites
    if ! command -v bitbake &>/dev/null; then
        log_warn "bitbake not found - cannot build Yocto image"
        return 1
    fi

    if [[ ! -d "${PROJECT_DIR}/linux/meta-mts" ]]; then
        log_error "meta-mts layer not found"
        return 1
    fi

    # Source Yocto environment
    log_info "Sourcing Yocto environment..."
    local poky_dir="${BUILD_DIR}/yocto-work/poky"
    if [[ -d "${poky_dir}" ]]; then
        source "${poky_dir}/oe-init-build-env" "${build_path}" 2>/dev/null || true
    else
        log_warn "Poky not found - run init-yocto-env.sh first"
        return 1
    fi

    # Add layers
    bitbake-layers add-layer ../../linux/meta-openembedded/meta-oe 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-openembedded/meta-networking 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-virtualization 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-kubernetes 2>/dev/null || true
    bitbake-layers add-layer ../../linux/meta-mts 2>/dev/null || true

    # Build
    log_info "Running: bitbake mts-mobile-core-image"
    if bitbake mts-mobile-core-image 2>&1 | tee "${log_file}"; then
        log_success "Yocto build for MC-5000 completed"
        if [[ "${VALIDATE}" = true ]]; then
            validate_yocto_image "MC5000" "${build_path}"
        fi
        return 0
    else
        log_error "Yocto build for MC-5000 failed - see ${log_file}"
        return 1
    fi
}

# Build Buildroot image for Mobile Backhaul
build_buildroot_mobile_backhaul() {
    log_section "Building Buildroot Image for MTS-MB-3000 (Mobile Backhaul)"

    local build_path="${BUILD_DIR}/mobile-backhaul/buildroot"
    mkdir -p "${build_path}"
    local log_file="${BUILD_LOGS_DIR}/build-mb3000.log"

    # Check prerequisites
    if ! command -v make &>/dev/null; then
        log_warn "make not found - cannot build Buildroot image"
        return 1
    fi

    local buildroot_dir="${BUILD_DIR}/buildroot-work/buildroot"
    if [[ ! -d "${buildroot_dir}" ]]; then
        log_warn "Buildroot not found - run init-buildroot.sh first"
        return 1
    fi

    # Copy defconfig and build
    log_info "Configuring Buildroot for MB-3000..."
    (
        cd "${buildroot_dir}"
        cp "${PROJECT_DIR}/linux/buildroot/mts-s32g3-defconfig" .config 2>/dev/null || true
        BR2_EXTERNAL="${PROJECT_DIR}/linux/meta-mts" make oldconfig 2>/dev/null || true
        log_info "Running: BR2_EXTERNAL=${PROJECT_DIR}/linux/meta-mts make"
        BR2_EXTERNAL="${PROJECT_DIR}/linux/meta-mts" make 2>&1 | tee "${log_file}"
    )

    if [[ $? -eq 0 ]]; then
        log_success "Buildroot build for MB-3000 completed"
        if [[ "${VALIDATE}" = true ]]; then
            validate_buildroot_image "MB3000" "${build_path}"
        fi
        return 0
    else
        log_error "Buildroot build for MB-3000 failed - see ${log_file}"
        return 1
    fi
}

# Build OpenWrt image for OLT GPON
build_openwrt_olt_gpon() {
    log_section "Building OpenWrt Image for MTS-OLT-2000 (OLT GPON)"

    local build_path="${BUILD_DIR}/olt-gpon/openwrt"
    mkdir -p "${build_path}"
    local log_file="${BUILD_LOGS_DIR}/build-olt2000.log"

    local openwrt_dir="${BUILD_DIR}/openwrt-work/openwrt"
    if [[ ! -d "${openwrt_dir}" ]]; then
        log_warn "OpenWrt not found - run init-openwrt-sdk.sh first"
        return 1
    fi

    (
        cd "${openwrt_dir}"
        log_info "Configuring OpenWrt for OLT GPON (armsr/armv10)..."
        make defconfig TARGET="armsr" SUBTARGET="armv10" 2>/dev/null || true
        log_info "Adding meta-mts-openwrt feeds..."
        if ! grep -q "src-git mts" feeds.conf.default 2>/dev/null; then
            echo "src-git mts ${PROJECT_DIR}/linux/meta-mts-openwrt" >> feeds.conf.default
        fi
        ./scripts/feeds update -a 2>/dev/null || true
        ./scripts/feeds install -a 2>/dev/null || true
        log_info "Running: make -j\$(nproc)"
        make -j"$(nproc)" 2>&1 | tee "${log_file}"
    )

    if [[ $? -eq 0 ]]; then
        log_success "OpenWrt build for OLT-2000 completed"
        if [[ "${VALIDATE}" = true ]]; then
            validate_openwrt_image "OLT2000" "${openwrt_dir}"
        fi
        return 0
    else
        log_error "OpenWrt build for OLT-2000 failed - see ${log_file}"
        return 1
    fi
}

# Build OpenWrt image for Enterprise Router
build_openwrt_enterprise() {
    log_section "Building OpenWrt Image for MTS-ER-1000 (Enterprise Router)"

    local build_path="${BUILD_DIR}/enterprise-router/openwrt"
    mkdir -p "${build_path}"
    local log_file="${BUILD_LOGS_DIR}/build-er1000.log"

    local openwrt_dir="${BUILD_DIR}/openwrt-work/openwrt"
    if [[ ! -d "${openwrt_dir}" ]]; then
        log_warn "OpenWrt not found - run init-openwrt-sdk.sh first"
        return 1
    fi

    (
        cd "${openwrt_dir}"
        log_info "Configuring OpenWrt for Enterprise (freescale/armv8)..."
        make defconfig TARGET="freescale" SUBTARGET="armv8" 2>/dev/null || true
        if ! grep -q "src-git mts" feeds.conf.default 2>/dev/null; then
            echo "src-git mts ${PROJECT_DIR}/linux/meta-mts-openwrt" >> feeds.conf.default
        fi
        ./scripts/feeds update -a 2>/dev/null || true
        ./scripts/feeds install -a 2>/dev/null || true
        log_info "Running: make -j\$(nproc)"
        make -j"$(nproc)" 2>&1 | tee "${log_file}"
    )

    if [[ $? -eq 0 ]]; then
        log_success "OpenWrt build for ER-1000 completed"
        if [[ "${VALIDATE}" = true ]]; then
            validate_openwrt_image "ER1000" "${openwrt_dir}"
        fi
        return 0
    else
        log_error "OpenWrt build for ER-1000 failed - see ${log_file}"
        return 1
    fi
}

# Build OpenWrt image for Residential Gateway
build_openwrt_residential() {
    log_section "Building OpenWrt Image for MTS-RG-500 (Residential Gateway)"

    local build_path="${BUILD_DIR}/residential-gateway/openwrt"
    mkdir -p "${build_path}"
    local log_file="${BUILD_LOGS_DIR}/build-rg500.log"

    local openwrt_dir="${BUILD_DIR}/openwrt-work/openwrt"
    if [[ ! -d "${openwrt_dir}" ]]; then
        log_warn "OpenWrt not found - run init-openwrt-sdk.sh first"
        return 1
    fi

    (
        cd "${openwrt_dir}"
        log_info "Configuring OpenWrt for Residential (mediatek/mt7981)..."
        make defconfig TARGET="mediatek" SUBTARGET="mt7981" 2>/dev/null || true
        if ! grep -q "src-git mts" feeds.conf.default 2>/dev/null; then
            echo "src-git mts ${PROJECT_DIR}/linux/meta-mts-openwrt" >> feeds.conf.default
        fi
        ./scripts/feeds update -a 2>/dev/null || true
        ./scripts/feeds install -a 2>/dev/null || true
        log_info "Running: make -j\$(nproc)"
        make -j"$(nproc)" 2>&1 | tee "${log_file}"
    )

    if [[ $? -eq 0 ]]; then
        log_success "OpenWrt build for RG-500 completed"
        if [[ "${VALIDATE}" = true ]]; then
            validate_openwrt_image "RG500" "${openwrt_dir}"
        fi
        return 0
    else
        log_error "OpenWrt build for RG-500 failed - see ${log_file}"
        return 1
    fi
}

# Clean build artifacts
clean_build() {
    log_section "Cleaning Build Artifacts"
    log_info "Cleaning build directory: ${BUILD_DIR}"
    if [[ -d "${BUILD_DIR}" ]]; then
        rm -rf "${BUILD_DIR:?}"/*
        log_success "Build directory cleaned"
    else
        log_info "Build directory does not exist"
    fi
}

# Main execution
main() {
    log_section "MTS Router OS Image Build System"
    echo ""
    echo -e "  Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo -e "  Host: $(hostname)"
    echo -e "  Build dir: ${BUILD_DIR}"
    echo -e "  Device: ${DEVICE}"
    echo -e "  Validate: ${VALIDATE}"
    echo ""

    # Clean if requested
    if [[ "${CLEAN}" == true ]]; then
        clean_build
        exit 0
    fi

    # Build based on device
    local success_count=0
    local fail_count=0

    build_device() {
        local device_name="$1"
        shift
        if "$@"; then
            success_count=$((success_count + 1))
        else
            fail_count=$((fail_count + 1))
        fi
    }

    case "${DEVICE}" in
        all)
            build_device "CR-9000" build_yocto_core_router || true
            build_device "MC-5000" build_yocto_mobile_core || true
            build_device "MB-3000" build_buildroot_mobile_backhaul || true
            build_device "OLT-2000" build_openwrt_olt_gpon || true
            build_device "ER-1000" build_openwrt_enterprise || true
            build_device "RG-500" build_openwrt_residential || true
            ;;
        cr9000)
            build_device "CR-9000" build_yocto_core_router
            ;;
        mc5000)
            build_device "MC-5000" build_yocto_mobile_core
            ;;
        mb3000)
            build_device "MB-3000" build_buildroot_mobile_backhaul
            ;;
        olt2000)
            build_device "OLT-2000" build_openwrt_olt_gpon
            ;;
        er1000)
            build_device "ER-1000" build_openwrt_enterprise
            ;;
        rg500)
            build_device "RG-500" build_openwrt_residential
            ;;
        *)
            log_error "Unknown device: ${DEVICE}"
            exit 1
            ;;
    esac

    # Summary
    log_section "Build Summary"
    echo ""
    echo -e "  ${GREEN}Builds completed: ${success_count} succeeded, ${fail_count} failed${NC}"
    echo ""
    echo "  Build logs: ${BUILD_LOGS_DIR}/"
    echo "  Results: ${RESULTS_FILE}"
    echo ""
    echo "  Completed: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo ""
}

# Run main
main "$@"
