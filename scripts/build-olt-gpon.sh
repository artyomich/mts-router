#!/bin/bash
# Build script for MTS-OLT-2000 OLT GPON (OpenWrt)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build/olt-gpon"
LOG_FILE="${BUILD_DIR}/build.log"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }

mkdir -p "${BUILD_DIR}"

log "=== Building MTS-OLT-2000 OLT GPON (OpenWrt) ==="
log "Build directory: ${BUILD_DIR}"

# ============================================================
# Step 1: Clone OpenWrt source
# ============================================================
log "Step 1: Setting up OpenWrt build environment..."

OPENWRT_VERSION="23.05.4"
OPENWRT_URL="https://github.com/openwrt/openwrt/releases/download/${OPENWRT_VERSION}/openwrt-sdk-${OPENWRT_VERSION}-x86-64-gcc-12.3.0_musl.Linux-x86_64.tar.xz"
OPENWRT_DIR="${BUILD_DIR}/openwrt"

if [ ! -d "${OPENWRT_DIR}" ]; then
    log "Downloading OpenWrt SDK ${OPENWRT_VERSION}..."
    mkdir -p "${BUILD_DIR}/downloads"
    
    # Use cached SDK if available
    if [ -f "${BUILD_DIR}/downloads/openwrt-sdk-${OPENWRT_VERSION}.tar.xz" ]; then
        log "Using cached SDK..."
        cp "${BUILD_DIR}/downloads/openwrt-sdk-${OPENWRT_VERSION}.tar.xz" "${BUILD_DIR}/"
    else
        curl -L -o "${BUILD_DIR}/downloads/openwrt-sdk-${OPENWRT_VERSION}.tar.xz" "${OPENWRT_URL}" || {
            log_error "Failed to download OpenWrt SDK"
            exit 1
        }
    fi
    
    log "Extracting SDK..."
    tar xf "${BUILD_DIR}/openwrt-sdk-${OPENWRT_VERSION}*.tar.xz" -C "${BUILD_DIR}/"
    mv "${BUILD_DIR}/openwrt-sdk-"* "${OPENWRT_DIR}"
else
    log "OpenWrt SDK already exists, skipping download"
fi

# ============================================================
# Step 2: Configure OpenWrt for MTS-OLT-2000
# ============================================================
log "Step 2: Configuring OpenWrt for MTS-OLT-2000..."

cd "${OPENWRT_DIR}"

# Update feeds
./scripts/feeds update -a 2>&1 | tee -a "${LOG_FILE}"
./scripts/feeds install -a 2>&1 | tee -a "${LOG_FILE}"

# Apply MTS-specific patches
if [ -d "${PROJECT_DIR}/olt-gpatchs" ]; then
    log "Applying MTS patches..."
    for patch in "${PROJECT_DIR}/olt-gpatchs/"*.patch; do
        if [ -f "${patch}" ]; then
            log "Applying ${patch}..."
            patch -p1 < "${patch}" 2>&1 | tee -a "${LOG_FILE}" || true
        fi
    done
fi

# Create MTS-OLT-2000 target configuration
cat > "${BUILD_DIR}/olt2000.config" << 'EOF'
# MTS-OLT-2000 OpenWrt Configuration
# Tofino 2 ASIC + AMD EPYC + RTL960x GPON

# Target system
CONFIG_TARGET_x86=y
CONFIG_TARGET_x86_64=y
CONFIG_TARGET_x86_64_GENERIC=y

# Package selection
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_luci-ssl=y
CONFIG_PACKAGE_luci-app-cwmp=y
CONFIG_PACKAGE_luci-app-omci=y
CONFIG_PACKAGE_luci-app-status=y
CONFIG_PACKAGE_luci-app-system=y
CONFIG_PACKAGE_luci-app-network=y
CONFIG_PACKAGE_luci-app-firewall=y

# GPON packages
CONFIG_PACKAGE_kmod-rtl960x=y
CONFIG_PACKAGE_omci-handler=y
CONFIG_PACKAGE_cwmpd=y
CONFIG_PACKAGE_omci-manager=y

# Network packages
CONFIG_PACKAGE_iproute2=y
CONFIG_PACKAGE_iproute2-ebpf=y
CONFIG_PACKAGE_ipset=y
CONFIG_PACKAGE_ip6tables=y
CONFIG_PACKAGE_iptables=y
CONFIG_PACKAGE_iptables-mod-nat=y
CONFIG_PACKAGE_iptables-mod-filter=y
CONFIG_PACKAGE_iptables-mod-tproxy=y
CONFIG_PACKAGE_iptables-mod-conntrack=y
CONFIG_PACKAGE_iptables-mod-conntrack-extra=y
CONFIG_PACKAGE_iptables-mod-ipopt=y
CONFIG_PACKAGE_iptables-mod-ipsec=y
CONFIG_PACKAGE_iptables-mod-raw=y
CONFIG_PACKAGE_iptables-mod-extra=y

# Routing packages
CONFIG_PACKAGE_frr=y
CONFIG_PACKAGE_frr-bgpd=y
CONFIG_PACKAGE_frr-ospfd=y
CONFIG_PACKAGE_frr-ospf6d=y
CONFIG_PACKAGE_frr-pimd=y
CONFIG_PACKAGE_frr-lldp=y
CONFIG_PACKAGE_frr-snmp=y

# Telemetry
CONFIG_PACKAGE_telegraf=y
CONFIG_PACKAGE_prometheus=y
CONFIG_PACKAGE_grafana=y

# VoIP
CONFIG_PACKAGE_asterisk=y
CONFIG_PACKAGE_asterisk-pjsip=y
CONFIG_PACKAGE_asterisk-core=y

# Development
CONFIG_PACKAGE_python3=y
CONFIG_PACKAGE_python3-pip=y
CONFIG_PACKAGE_python3-requests=y
CONFIG_PACKAGE_python3-scapy=y
EOF

# Copy config and build
cp "${BUILD_DIR}/olt2000.config" "${OPENWRT_DIR}/.config"

# Menuconfig to validate
make defconfig 2>&1 | tee -a "${LOG_FILE}"

# ============================================================
# Step 3: Build OpenWrt image
# ============================================================
log "Step 3: Building OpenWrt image..."

make -j$(nproc) 2>&1 | tee -a "${LOG_FILE}" || {
    log_error "OpenWrt build failed"
    exit 1
}

# ============================================================
# Step 4: Verify build artifacts
# ============================================================
log "Step 4: Verifying build artifacts..."

DEPLOY_DIR="${BUILD_DIR}/images"
mkdir -p "${DEPLOY_DIR}"

# Find and copy images
if [ -d "${OPENWRT_DIR}/bin/targets" ]; then
    find "${OPENWRT_DIR}/bin/targets" -type f \( -name "*.img.gz" -o -name "*.bin" -o -name "*.squashfs" \) | while read -r img; do
        basename_img=$(basename "${img}")
        cp "${img}" "${DEPLOY_DIR}/mts-olt2000-${basename_img}"
        log "Copied: ${img} -> ${DEPLOY_DIR}/mts-olt2000-${basename_img}"
    done
fi

# Generate checksums
if [ -d "${DEPLOY_DIR}" ]; then
    cd "${DEPLOY_DIR}"
    for img in mts-olt2000-*; do
        if [ -f "${img}" ]; then
            sha256sum "${img}" > "${img}.sha256"
            log "Checksum: ${img}.sha256"
        fi
    done
fi

# ============================================================
# Step 5: Generate build manifest
# ============================================================
log "Step 5: Generating build manifest..."

cat > "${DEPLOY_DIR}/mts-olt2000-manifest.txt" << EOF
# MTS-OLT-2000 OpenWrt Build Manifest
# Build date: $(date -u +%Y-%m-%dT%H:%M:%SZ)
# OpenWrt version: ${OPENWRT_VERSION}
# Target: x86/64 generic
# Configuration: ${BUILD_DIR}/olt2000.config

## Packages installed:
$(grep "^CONFIG_PACKAGE_" "${BUILD_DIR}/olt2000.config" | sed 's/CONFIG_PACKAGE_//;s/=y//' | sort -u)

## Kernel version:
$(grep "^CONFIG_KERNEL_" "${OPENWRT_DIR}/.config" 2>/dev/null | grep "=y" | head -20 || echo "N/A")

## Toolchain:
GCC: $(gcc --version 2>/dev/null | head -1 || echo "N/A")
Musl: $(ls "${OPENWRT_DIR}/staging_dir/toolchain-"* | head -1 || echo "N/A")
EOF

# ============================================================
# Done
# ============================================================
log "=== MTS-OLT-2000 OpenWrt build complete ==="
log "Images: ${DEPLOY_DIR}/"
log "Log: ${LOG_FILE}"
log "Manifest: ${DEPLOY_DIR}/mts-olt2000-manifest.txt"

exit 0
