#!/bin/bash
# Build script for MTS-RG-500 Residential Gateway (OpenWrt)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build/residential"
LOG_FILE="${BUILD_DIR}/build.log"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }

mkdir -p "${BUILD_DIR}"

log "=== Building MTS-RG-500 Residential Gateway (OpenWrt) ==="
log "Build directory: ${BUILD_DIR}"

# ============================================================
# Step 1: Clone OpenWrt source
# ============================================================
log "Step 1: Setting up OpenWrt build environment..."

OPENWRT_VERSION="23.05.4"
OPENWRT_URL="https://github.com/openwrt/openwrt/releases/download/${OPENWRT_VERSION}/openwrt-sdk-mediatek_mt7981-gcc-12.3.0_musl.Linux-x86_64.tar.xz"
OPENWRT_DIR="${BUILD_DIR}/openwrt"

if [ ! -d "${OPENWRT_DIR}" ]; then
    log "Downloading OpenWrt SDK ${OPENWRT_VERSION}..."
    mkdir -p "${BUILD_DIR}/downloads"
    
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
# Step 2: Configure OpenWrt for MTS-RG-500
# ============================================================
log "Step 2: Configuring OpenWrt for MTS-RG-500..."

cd "${OPENWRT_DIR}"

./scripts/feeds update -a 2>&1 | tee -a "${LOG_FILE}"
./scripts/feeds install -a 2>&1 | tee -a "${LOG_FILE}"

# Create MTS-RG-500 target configuration
cat > "${BUILD_DIR}/rg500.config" << 'EOF'
# MTS-RG-500 OpenWrt Configuration
# MediaTek MT7981 ARM64 + RTL960x GPON

# Target system
CONFIG_TARGET_mediatek=y
CONFIG_TARGET_mediatek_mt7981=y
CONFIG_TARGET_mediatek_mt7981_DEFAULT=y

# WiFi 6 (MT76)
CONFIG_PACKAGE_kmod-mt7981-wifi=y
CONFIG_PACKAGE_kmod-mt76-core=y
CONFIG_PACKAGE_kmod-mt76-connac=y
CONFIG_PACKAGE_kmod-mt76-connac3=y
CONFIG_PACKAGE_kmod-mt76-efusa=y
CONFIG_PACKAGE_kmod-mt76-eta=y
CONFIG_PACKAGE_kmod-mt7915e=y
CONFIG_PACKAGE_kmod-mt7981-eth=y
CONFIG_PACKAGE_kmod-mt7530=y
CONFIG_PACKAGE_luci-app-mtwifi=y

# GPON (RTL960x)
CONFIG_PACKAGE_kmod-rtl960x=y
CONFIG_PACKAGE_omci-handler=y
CONFIG_PACKAGE_omci-manager=y

# VoIP (Asterisk)
CONFIG_PACKAGE_asterisk=y
CONFIG_PACKAGE_asterisk-pjsip=y
CONFIG_PACKAGE_asterisk-core=y
CONFIG_PACKAGE_asterisk-core-apps=y
CONFIG_PACKAGE_asterisk-core-modules=y
CONFIG_PACKAGE_asterisk-codecs=y
CONFIG_PACKAGE_asterisk-channels=y
CONFIG_PACKAGE_asterisk-res-audio=y
CONFIG_PACKAGE_asterisk-res-fax=y
CONFIG_PACKAGE_asterisk-res-speech=y
CONFIG_PACKAGE_asterisk-res-ssl=y
CONFIG_PACKAGE_asterisk-res-timing=y

# IPTV
CONFIG_PACKAGE_igmpproxy=y
CONFIG_PACKAGE_ppp=y
CONFIG_PACKAGE_ppp-mod-pppoe=y
CONFIG_PACKAGE_ppp-mod-pap=y
CONFIG_PACKAGE_ppp-mod-chap=y
CONFIG_PACKAGE_iptables-mod-nat-extra=y
CONFIG_PACKAGE_kmod-ipt-extra=y

# TR-069
CONFIG_PACKAGE_cwmpd=y
CONFIG_PACKAGE_luci-app-cwmp=y

# Management
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_luci-ssl=y
CONFIG_PACKAGE_luci-app-status=y
CONFIG_PACKAGE_luci-app-system=y
CONFIG_PACKAGE_luci-app-network=y
CONFIG_PACKAGE_luci-app-firewall=y
CONFIG_PACKAGE_luci-app-ddns=y
CONFIG_PACKAGE_luci-app-upnp=y
CONFIG_PACKAGE_luci-app-samba=y
CONFIG_PACKAGE_luci-app-htop=y

# Network
CONFIG_PACKAGE_iproute2=y
CONFIG_PACKAGE_ipset=y
CONFIG_PACKAGE_iptables=y
CONFIG_PACKAGE_iptables-mod-nat=y
CONFIG_PACKAGE_iptables-mod-conntrack=y
CONFIG_PACKAGE_ip6tables=y
CONFIG_PACKAGE_mtr=y
CONFIG_PACKAGE_tcpdump=y

# WiFi management
CONFIG_PACKAGE_hostapd-common=y
CONFIG_PACKAGE_hostapd-phy0=y
CONFIG_PACKAGE_hostapd-phy1=y
CONFIG_PACKAGE_wpad-basic=y
CONFIG_PACKAGE_wifi-scripts=y
CONFIG_PACKAGE_ubus=y
CONFIG_PACKAGE_ubusd=y
CONFIG_PACKAGE_uclient-fetch=y
CONFIG_PACKAGE_libubox=y
CONFIG_PACKAGE_libubus=y
CONFIG_PACKAGE_libuci=y
CONFIG_PACKAGE_libustream-mbedtls=y
CONFIG_PACKAGE_libmbedtls=y
CONFIG_PACKAGE_libopenssl=y
CONFIG_PACKAGE_libzlib=y

# Telemetry
CONFIG_PACKAGE_telegraf=y
CONFIG_PACKAGE_prometheus=y
CONFIG_PACKAGE_grafana=y

# Development
CONFIG_PACKAGE_python3=y
CONFIG_PACKAGE_python3-pip=y
CONFIG_PACKAGE_python3-requests=y
CONFIG_PACKAGE_python3-scapy=y
EOF

cp "${BUILD_DIR}/rg500.config" "${OPENWRT_DIR}/.config"
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

if [ -d "${OPENWRT_DIR}/bin/targets" ]; then
    find "${OPENWRT_DIR}/bin/targets" -type f \( -name "*.img.gz" -o -name "*.bin" -o -name "*.squashfs" \) | while read -r img; do
        basename_img=$(basename "${img}")
        cp "${img}" "${DEPLOY_DIR}/mts-rg500-${basename_img}"
        log "Copied: ${img} -> ${DEPLOY_DIR}/mts-rg500-${basename_img}"
    done
fi

if [ -d "${DEPLOY_DIR}" ]; then
    cd "${DEPLOY_DIR}"
    for img in mts-rg500-*; do
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

cat > "${DEPLOY_DIR}/mts-rg500-manifest.txt" << EOF
# MTS-RG-500 OpenWrt Build Manifest
# Build date: $(date -u +%Y-%m-%dT%H:%M:%SZ)
# OpenWrt version: ${OPENWRT_VERSION}
# Target: mediatek/mt7981
# Configuration: ${BUILD_DIR}/rg500.config
EOF

log "=== MTS-RG-500 OpenWrt build complete ==="
log "Images: ${DEPLOY_DIR}/"
log "Log: ${LOG_FILE}"

exit 0
