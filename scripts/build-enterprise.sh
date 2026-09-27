#!/bin/bash
# Build script for MTS-ER-1000 Enterprise Router (OpenWrt)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build/enterprise"
LOG_FILE="${BUILD_DIR}/build.log"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }

mkdir -p "${BUILD_DIR}"

log "=== Building MTS-ER-1000 Enterprise Router (OpenWrt) ==="
log "Build directory: ${BUILD_DIR}"

# ============================================================
# Step 1: Clone OpenWrt source
# ============================================================
log "Step 1: Setting up OpenWrt build environment..."

OPENWRT_VERSION="23.05.4"
OPENWRT_URL="https://github.com/openwrt/openwrt/releases/download/${OPENWRT_VERSION}/openwrt-sdk-${OPENWRT_VERSION}-arm_armv7-vfpv3-gcc-12.3.0_musl.Linux-x86_64.tar.xz"
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
# Step 2: Configure OpenWrt for MTS-ER-1000
# ============================================================
log "Step 2: Configuring OpenWrt for MTS-ER-1000..."

cd "${OPENWRT_DIR}"

./scripts/feeds update -a 2>&1 | tee -a "${LOG_FILE}"
./scripts/feeds install -a 2>&1 | tee -a "${LOG_FILE}"

# Create MTS-ER-1000 target configuration
cat > "${BUILD_DIR}/er1000.config" << 'EOF'
# MTS-ER-1000 OpenWrt Configuration
# NXP S32G3 ARM32 + TomTom switch

# Target system
CONFIG_TARGET_arm=y
CONFIG_TARGET_arm_armv7=y
CONFIG_TARGET_arm_armv7_vfpv3=y
CONFIG_TARGET_arm_armv7_vfpv3_NXP_S32G=y

# SD-WAN packages
CONFIG_PACKAGE_sdwan-engine=y
CONFIG_PACKAGE_sdwan-manager=y
CONFIG_PACKAGE_sdwan-path-selector=y
CONFIG_PACKAGE_sdwan-policy-controller=y
CONFIG_PACKAGE_sdwan-monitor=y
CONFIG_PACKAGE_sdwan-health-check=y
CONFIG_PACKAGE_sdwan-failover=y
CONFIG_PACKAGE_sdwan-load-balancer=y

# IPSec packages
CONFIG_PACKAGE_strongswan=y
CONFIG_PACKAGE_strongswan-swanctl=y
CONFIG_PACKAGE_strongswan-charon=y
CONFIG_PACKAGE_strongswan-ipsecctl=y
CONFIG_PACKAGE_strongswan-mod-crypto-aes=y
CONFIG_PACKAGE_strongswan-mod-crypto-sha2=y
CONFIG_PACKAGE_strongswan-mod-crypto-sha1=y
CONFIG_PACKAGE_strongswan-mod-crypto-md5=y
CONFIG_PACKAGE_strongswan-mod-crypto-des=y
CONFIG_PACKAGE_strongswan-mod-crypto-aesni=y
CONFIG_PACKAGE_strongswan-mod-crypto-camellia=y
CONFIG_PACKAGE_strongswan-mod-crypto-blowfish=y
CONFIG_PACKAGE_strongswan-mod-crypto-chaCha20=y
CONFIG_PACKAGE_strongswan-mod-crypto-poly1305=y
CONFIG_PACKAGE_strongswan-mod-crypto-gcm=y
CONFIG_PACKAGE_strongswan-mod-crypto-ccm=y
CONFIG_PACKAGE_strongswan-mod-crypto-ecdsa=y
CONFIG_PACKAGE_strongswan-mod-crypto-eddsa=y
CONFIG_PACKAGE_strongswan-mod-crypto-ecdh=y
CONFIG_PACKAGE_strongswan-mod-crypto-dh=y
CONFIG_PACKAGE_strongswan-mod-crypto-prf=y
CONFIG_PACKAGE_strongswan-mod-crypto-prfplus=y
CONFIG_PACKAGE_strongswan-mod-crypto-kdf=y
CONFIG_PACKAGE_strongswan-mod-ipsec=y
CONFIG_PACKAGE_strongswan-mod-addrblock=y
CONFIG_PACKAGE_strongswan-mod-unity=y
CONFIG_PACKAGE_strongswan-mod-csv=y
CONFIG_PACKAGE_strongswan-mod-dbg=y
CONFIG_PACKAGE_strongswan-mod-eap-identity=y
CONFIG_PACKAGE_strongswan-mod-eap-md5=y
CONFIG_PACKAGE_strongswan-mod-eap-mschapv2=y
CONFIG_PACKAGE_strongswan-mod-eap-peap=y
CONFIG_PACKAGE_strongswan-mod-eap-tls=y
CONFIG_PACKAGE_strongswan-mod-eap-ttls=y
CONFIG_PACKAGE_strongswan-mod-eap-psk=y
CONFIG_PACKAGE_strongswan-mod-eap-dynamic=y
CONFIG_PACKAGE_strongswan-mod-eap-aeskey=y
CONFIG_PACKAGE_strongswan-mod-fips=y
CONFIG_PACKAGE_strongswan-mod-ipseckey=y
CONFIG_PACKAGE_strongswan-mod-kernel-libipsec=y
CONFIG_PACKAGE_strongswan-mod-kernel-netlink=y
CONFIG_PACKAGE_strongswan-mod-leafdb=y
CONFIG_PACKAGE_strongswan-mod-led=y
CONFIG_PACKAGE_strongswan-mod-lookup=y
CONFIG_PACKAGE_strongswan-mod-mysql=y
CONFIG_PACKAGE_strongswan-mod-nss=y
CONFIG_PACKAGE_strongswan-mod-pkcs11=y
CONFIG_PACKAGE_strongswan-mod-pubkey=y
CONFIG_PACKAGE_strongswan-mod-radius=y
CONFIG_PACKAGE_strongswan-mod-soup=y
CONFIG_PACKAGE_strongswan-mod-sqlite=y
CONFIG_PACKAGE_strongswan-mod-sockaddr=y
CONFIG_PACKAGE_strongswan-mod-socket2=y
CONFIG_PACKAGE_strongswan-mod-ssl=y
CONFIG_PACKAGE_strongswan-mod-tpm=y
CONFIG_PACKAGE_strongswan-mod-unique-id=y
CONFIG_PACKAGE_strongswan-mod-updown=y
CONFIG_PACKAGE_strongswan-mod-vici=y
CONFIG_PACKAGE_strongswan-mod-x509=y
CONFIG_PACKAGE_strongswan-mod-xauth-generic=y
CONFIG_PACKAGE_strongswan-mod-xauth-esm=y
CONFIG_PACKAGE_strongswan-mod-xauth-pam=y

# Routing
CONFIG_PACKAGE_frr=y
CONFIG_PACKAGE_frr-bgpd=y
CONFIG_PACKAGE_frr-ospfd=y
CONFIG_PACKAGE_frr-ospf6d=y
CONFIG_PACKAGE_frr-bmp=y

# TomTom switch support
CONFIG_PACKAGE_kmod-tomtom-switch=y
CONFIG_PACKAGE_kmod-tomtom-switch-mac=y
CONFIG_PACKAGE_kmod-tomtom-switch-phy=y
CONFIG_PACKAGE_kmod-tomtom-switch-vlan=y
CONFIG_PACKAGE_kmod-tomtom-switch-qos=y

# Network
CONFIG_PACKAGE_iproute2=y
CONFIG_PACKAGE_ipset=y
CONFIG_PACKAGE_iptables=y
CONFIG_PACKAGE_iptables-mod-nat=y
CONFIG_PACKAGE_iptables-mod-filter=y
CONFIG_PACKAGE_iptables-mod-conntrack=y
CONFIG_PACKAGE_ip6tables=y
CONFIG_PACKAGE_mtr=y
CONFIG_PACKAGE_tcpdump=y

# Telemetry
CONFIG_PACKAGE_telegraf=y
CONFIG_PACKAGE_prometheus=y
CONFIG_PACKAGE_grafana=y

# Management
CONFIG_PACKAGE_cwmpd=y
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_luci-ssl=y
CONFIG_PACKAGE_luci-app-sdwan=y
CONFIG_PACKAGE_luci-app-status=y
CONFIG_PACKAGE_luci-app-system=y
CONFIG_PACKAGE_luci-app-network=y
CONFIG_PACKAGE_luci-app-firewall=y
EOF

cp "${BUILD_DIR}/er1000.config" "${OPENWRT_DIR}/.config"
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
        cp "${img}" "${DEPLOY_DIR}/mts-er1000-${basename_img}"
        log "Copied: ${img} -> ${DEPLOY_DIR}/mts-er1000-${basename_img}"
    done
fi

if [ -d "${DEPLOY_DIR}" ]; then
    cd "${DEPLOY_DIR}"
    for img in mts-er1000-*; do
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

cat > "${DEPLOY_DIR}/mts-er1000-manifest.txt" << EOF
# MTS-ER-1000 OpenWrt Build Manifest
# Build date: $(date -u +%Y-%m-%dT%H:%M:%SZ)
# OpenWrt version: ${OPENWRT_VERSION}
# Target: arm/armv7 NXP S32G
# Configuration: ${BUILD_DIR}/er1000.config
EOF

log "=== MTS-ER-1000 OpenWrt build complete ==="
log "Images: ${DEPLOY_DIR}/"
log "Log: ${LOG_FILE}"

exit 0
