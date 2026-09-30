#!/bin/bash
# Yocto/Buildroot/OpenWrt Build Orchestration Script
# 
# This script orchestrates building OS images for all MTS Router devices
# using their respective build systems:
#   - MTS-CR-9000, MTS-MC-5000: Yocto (meta-mts layer)
#   - MTS-MB-3000: Buildroot
#   - MTS-OLT-2000, MTS-ER-1000, MTS-RG-500: OpenWrt
#
# Usage:
#   ./build-yocto-image.sh [--device <device>] [--clean] [--help]
#
# Prerequisites:
#   For Yocto: bitbake, meta-mts layer, target machine configs
#   For Buildroot: make, buildroot tree
#   For OpenWrt: openwrt tree, toolchain

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Default settings
DEVICE="all"
CLEAN=false
VERBOSE=false
BUILD_DIR="${PROJECT_DIR}/build"

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
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--device <device>] [--clean] [--verbose]"
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
            echo "  --verbose     Show detailed build output"
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
log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

log_section() {
    echo ""
    echo -e "${CYAN}========================================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================================${NC}"
}

# Check if required commands are available
check_command() {
    local cmd="$1"
    local desc="$2"
    
    if command -v "$cmd" &>/dev/null; then
        log_info "${cmd} found - ${desc}"
        return 0
    else
        log_warn "${cmd} NOT found - ${desc}"
        return 1
    fi
}

# Check Yocto prerequisites
check_yocto_prerequisites() {
    log_info "Checking Yocto prerequisites..."
    
    local missing=0
    
    if ! check_command "bitbake" "Yocto build tool"; then
        missing=$((missing + 1))
    fi
    
    if [[ -d "${PROJECT_DIR}/linux/meta-mts" ]]; then
        log_info "meta-mts layer found"
    else
        log_warn "meta-mts layer not found at ${PROJECT_DIR}/linux/meta-mts"
        missing=$((missing + 1))
    fi
    
    if [[ $missing -gt 0 ]]; then
        log_warn "Yocto prerequisites missing - cannot build Yocto images"
        return 1
    fi
    
    return 0
}

# Check Buildroot prerequisites
check_buildroot_prerequisites() {
    log_info "Checking Buildroot prerequisites..."
    
    if check_command "make" "Buildroot build tool"; then
        return 0
    else
        log_warn "Buildroot prerequisites missing"
        return 1
    fi
}

# Check OpenWrt prerequisites
check_openwrt_prerequisites() {
    log_info "Checking OpenWrt prerequisites..."
    
    local missing=0
    
    if ! check_command "make" "OpenWrt build tool"; then
        missing=$((missing + 1))
    fi
    
    if ! check_command "gcc" "C compiler"; then
        missing=$((missing + 1))
    fi
    
    if [[ $missing -gt 0 ]]; then
        log_warn "OpenWrt prerequisites missing"
        return 1
    fi
    
    return 0
}

# Build Yocto image for Core Router
build_yocto_core_router() {
    log_section "Building Yocto Image for MTS-CR-9000 (Core Router)"
    
    local build_path="${BUILD_DIR}/core-router/yocto"
    mkdir -p "${build_path}"
    
    # Check prerequisites
    if ! check_yocto_prerequisites; then
        log_warn "Skipping Yocto build for CR-9000 (prerequisites not met)"
        return 1
    fi
    
    # Source Yocto environment
    log_info "Sourcing Yocto environment..."
    if [[ -f "${PROJECT_DIR}/linux/meta-mts/oe-init-build-env" ]]; then
        source "${PROJECT_DIR}/linux/meta-mts/oe-init-build-env" "${build_path}" 2>/dev/null || true
    fi
    
    # Check for machine configuration
    local machine_conf="${PROJECT_DIR}/linux/meta-mts/conf/machine/mts-cr9000.conf"
    if [[ -f "${machine_conf}" ]]; then
        log_info "Machine config found: ${machine_conf}"
    else
        log_warn "Machine config not found: ${machine_conf}"
    fi
    
    # Check for image recipe
    local image_recipe="${PROJECT_DIR}/linux/meta-mts/recipes-core/images/mts-core-router-image.bb"
    if [[ -f "${image_recipe}" ]]; then
        log_info "Image recipe found: ${image_recipe}"
    else
        log_warn "Image recipe not found: ${image_recipe}"
    fi
    
    # Build command (would be: bitbake mts-core-router-image)
    log_info "Build command (requires Yocto environment):"
    log_info "  cd ${build_path}"
    log_info "  bitbake mts-core-router-image"
    
    # Show what would be built
    log_info "Expected outputs:"
    log_info "  - ${build_path}/tmp/deploy/images/mts-cr9000/*.ext4"
    log_info "  - ${build_path}/tmp/deploy/images/mts-cr9000/*.wic"
    log_info "  - ${build_path}/tmp/deploy/images/mts-cr9000/zImage"
    log_info "  - ${build_path}/tmp/deploy/images/mts-cr9000/*.dtb"
    
    return 0
}

# Build Yocto image for Mobile Core
build_yocto_mobile_core() {
    log_section "Building Yocto + K3s Image for MTS-MC-5000 (Mobile Core)"
    
    local build_path="${BUILD_DIR}/mobile-core/yocto"
    mkdir -p "${build_path}"
    
    # Check prerequisites
    if ! check_yocto_prerequisites; then
        log_warn "Skipping Yocto build for MC-5000 (prerequisites not met)"
        return 1
    fi
    
    log_info "Building Yocto image with K3s for Mobile Core..."
    log_info "Build command (requires Yocto environment):"
    log_info "  cd ${build_path}"
    log_info "  bitbake mts-mobile-core-image"
    
    # Show what would be built
    log_info "Expected outputs:"
    log_info "  - ${build_path}/tmp/deploy/images/mts-mc5000/*.ext4"
    log_info "  - ${build_path}/tmp/deploy/images/mts-mc5000/k3s-runtime.tar"
    log_info "  - ${build_path}/tmp/deploy/images/mts-mc5000/k3s-extensions.tar"
    
    return 0
}

# Build Buildroot image for Mobile Backhaul
build_buildroot_mobile_backhaul() {
    log_section "Building Buildroot Image for MTS-MB-3000 (Mobile Backhaul)"
    
    local build_path="${BUILD_DIR}/mobile-backhaul/buildroot"
    mkdir -p "${build_path}"
    
    # Check prerequisites
    if ! check_buildroot_prerequisites; then
        log_warn "Skipping Buildroot build for MB-3000 (prerequisites not met)"
        return 1
    fi
    
    log_info "Configuring Buildroot for Mobile Backhaul..."
    log_info "Build command:"
    log_info "  cd ${build_path}"
    log_info "  make mts-mb3000_defconfig"
    log_info "  make"
    
    # Show what would be built
    log_info "Expected outputs:"
    log_info "  - ${build_path}/output/images/rootfs.tar"
    log_info "  - ${build_path}/output/images/rootfs.ext4"
    log_info "  - ${build_path}/output/images/zImage"
    log_info "  - ${build_path}/output/images/zynq-mts-mb3000.dtb"
    
    return 0
}

# Build OpenWrt image for OLT GPON
build_openwrt_olt_gpon() {
    log_section "Building OpenWrt Image for MTS-OLT-2000 (OLT GPON)"
    
    local build_path="${BUILD_DIR}/olt-gpon/openwrt"
    mkdir -p "${build_path}"
    
    # Check prerequisites
    if ! check_openwrt_prerequisites; then
        log_warn "Skipping OpenWrt build for OLT-2000 (prerequisites not met)"
        return 1
    fi
    
    log_info "Configuring OpenWrt for OLT GPON..."
    log_info "Build command:"
    log_info "  cd ${build_path}"
    log_info "  make defconfig"
    log_info "  make menuconfig  # Select MTS-OLT-2000 target"
    log_info "  make -j\$(nproc)"
    
    # Show what would be built
    log_info "Expected outputs:"
    log_info "  - ${build_path}/bin/targets/*/*-rootfs.tar.gz"
    log_info "  - ${build_path}/bin/targets/*/*-factory.bin"
    log_info "  - ${build_path}/bin/targets/*/*-sysupgrade.bin"
    
    return 0
}

# Build OpenWrt image for Enterprise Router
build_openwrt_enterprise() {
    log_section "Building OpenWrt Image for MTS-ER-1000 (Enterprise Router)"
    
    local build_path="${BUILD_DIR}/enterprise-router/openwrt"
    mkdir -p "${build_path}"
    
    # Check prerequisites
    if ! check_openwrt_prerequisites; then
        log_warn "Skipping OpenWrt build for ER-1000 (prerequisites not met)"
        return 1
    fi
    
    log_info "Configuring OpenWrt for Enterprise Router..."
    log_info "Build command:"
    log_info "  cd ${build_path}"
    log_info "  make defconfig"
    log_info "  make menuconfig  # Select MTS-ER-1000 target"
    log_info "  make -j\$(nproc)"
    
    log_info "Expected outputs:"
    log_info "  - ${build_path}/bin/targets/*/*-rootfs.tar.gz"
    log_info "  - ${build_path}/bin/targets/*/*-sysupgrade.bin"
    
    return 0
}

# Build OpenWrt image for Residential Gateway
build_openwrt_residential() {
    log_section "Building OpenWrt Image for MTS-RG-500 (Residential Gateway)"
    
    local build_path="${BUILD_DIR}/residential-gateway/openwrt"
    mkdir -p "${build_path}"
    
    # Check prerequisites
    if ! check_openwrt_prerequisites; then
        log_warn "Skipping OpenWrt build for RG-500 (prerequisites not met)"
        return 1
    fi
    
    log_info "Configuring OpenWrt for Residential Gateway..."
    log_info "Build command:"
    log_info "  cd ${build_path}"
    log_info "  make defconfig"
    log_info "  make menuconfig  # Select MTS-RG-500 target"
    log_info "  make -j\$(nproc)"
    
    log_info "Expected outputs:"
    log_info "  - ${build_path}/bin/targets/*/*-rootfs.tar.gz"
    log_info "  - ${build_path}/bin/targets/*/*-sysupgrade.bin"
    
    return 0
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
    echo "  Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "  Host: $(hostname)"
    echo "  Build dir: ${BUILD_DIR}"
    echo ""
    
    # Clean if requested
    if [[ "${CLEAN}" == true ]]; then
        clean_build
        return 0
    fi
    
    # Build based on device
    case "${DEVICE}" in
        all)
            build_yocto_core_router || true
            build_yocto_mobile_core || true
            build_buildroot_mobile_backhaul || true
            build_openwrt_olt_gpon || true
            build_openwrt_enterprise || true
            build_openwrt_residential || true
            ;;
        cr9000)
            build_yocto_core_router
            ;;
        mc5000)
            build_yocto_mobile_core
            ;;
        mb3000)
            build_buildroot_mobile_backhaul
            ;;
        olt2000)
            build_openwrt_olt_gpon
            ;;
        er1000)
            build_openwrt_enterprise
            ;;
        rg500)
            build_openwrt_residential
            ;;
        *)
            log_error "Unknown device: ${DEVICE}"
            exit 1
            ;;
    esac
    
    # Summary
    log_section "Build Summary"
    echo ""
    echo -e "  ${GREEN}Build orchestration complete${NC}"
    echo ""
    echo "  NOTE: Actual Yocto/Buildroot/OpenWrt builds require:"
    echo "    - Yocto: bitbake, meta-mts layer, target SDK"
    echo "    - Buildroot: buildroot tree, target toolchain"
    echo "    - OpenWrt: openwrt tree, target SDK"
    echo ""
    echo "  To perform actual builds, run the build commands shown above"
    echo "  in a system with the required build environment installed."
    echo ""
    echo "  Completed: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
}

# Run main
main "$@"
