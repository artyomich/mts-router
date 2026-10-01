#!/bin/bash
# init-openwrt-sdk.sh — Инициализация OpenWrt SDK для MTS Router
#
# Usage:
#   ./init-openwrt-sdk.sh [--clean] [--target <target>] [--help]
#
# Автоматически клонирует OpenWrt, настраивает target profiles для
# MTS-OLT-2000, MTS-ER-1000 и MTS-RG-500.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
META_MTS_OPENWRT="${PROJECT_DIR}/linux/meta-mts-openwrt"
WORK_DIR="${PROJECT_DIR}/build/openwrt-work"
CLEAN=false
TARGETS=("all")

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
Usage: $(basename "$0") [OPTIONS] [TARGETS...]

Initialize OpenWrt SDK for MTS Router devices.

Options:
  --clean                Remove existing openwrt and build directories
  --target <target>      Specific target to configure (armsr, freescale, mediatek)
  --help                 Show this help message

Targets:
  all                    All targets (default)
  armsr                  MTS-OLT-2000 (ARM64, RTL960x + Tofino 2)
  freescale              MTS-ER-1000 (ARM64, S32G3 + TomTom)
  mediatek               MTS-RG-500 (ARM64, MT7981 + RTL960x)

Examples:
  $(basename "$0")
  $(basename "$0") --target armsr
  $(basename "$0") --clean all
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --target)
            TARGETS=("$2")
            shift 2
            ;;
        --help|-h)
            usage
            ;;
        *)
            TARGETS+=("$1")
            shift
            ;;
    esac
done

# Step 1: Clean if requested
if [ "$CLEAN" = true ]; then
    log_info "Cleaning existing build environment..."
    rm -rf "${WORK_DIR}/openwrt"
    rm -rf "${WORK_DIR}/build"
    log_success "Clean completed"
fi

# Step 2: Clone OpenWrt if not exists
OPENWRT_DIR="${WORK_DIR}/openwrt"
if [ ! -d "${OPENWRT_DIR}/.git" ]; then
    log_info "Cloning OpenWrt trunk..."
    mkdir -p "${WORK_DIR}"
    git clone --depth 1 https://git.openwrt.org/openwrt/openwrt.git "${OPENWRT_DIR}"
    log_success "OpenWrt cloned successfully"
else
    log_info "OpenWrt already exists, skipping clone"
fi

# Step 3: Verify meta-mts-openwrt layer
if [ ! -d "${META_MTS_OPENWRT}" ]; then
    log_error "meta-mts-openwrt layer not found at ${META_MTS_OPENWRT}"
    exit 1
fi

# Step 4: Create build directory structure
mkdir -p "${WORK_DIR}/build"
log_success "Build directories created"

# Step 5: Generate setup script
SETUP_SCRIPT="${WORK_DIR}/setup-env.sh"
cat > "${SETUP_SCRIPT}" <<'SETUP_EOF'
#!/bin/bash
# Auto-generated setup script for OpenWrt environment

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "${SCRIPT_DIR}")"
OPENWRT_DIR="${SCRIPT_DIR}/openwrt"
META_LAYER="${PROJECT_DIR}/linux/meta-mts-openwrt"

cd "${OPENWRT_DIR}"

echo "OpenWrt environment ready."
echo "Meta layer: ${META_LAYER}"
echo ""
echo "To configure and build:"
echo ""
echo "  # MTS-OLT-2000 (armsr/armv10)"
echo "  make defconfig TARGET=armsr/subtarget=generic"
echo "  make menuconfig  # Add custom feeds"
echo "  make -j\$(nproc)"
echo ""
echo "  # MTS-ER-1000 (freescale/armv8)"
echo "  make defconfig TARGET=freescale/subtarget=generic"
echo "  make menuconfig  # Add custom feeds"
echo "  make -j\$(nproc)"
echo ""
echo "  # MTS-RG-500 (mediatek/mt7981)"
echo "  make defconfig TARGET=mediatek/subtarget=mt7981"
echo "  make menuconfig  # Add custom feeds"
echo "  make -j\$(nproc)"
echo ""
echo "To add custom feeds:"
echo "  echo 'src-git mts ${META_LAYER}' >> feeds.conf.default"
echo "  ./scripts/feeds update -a"
echo "  ./scripts/feeds install -a"
SETUP_EOF
chmod +x "${SETUP_SCRIPT}"
log_success "Setup script created at ${SETUP_SCRIPT}"

# Step 6: Create feeds configuration for meta-mts-openwrt
FEEDS_CONF="${OPENWRT_DIR}/feeds.conf.default"
if [ -f "${FEEDS_CONF}" ]; then
    if ! grep -q "src-git mts" "${FEEDS_CONF}" 2>/dev/null; then
        echo "src-git mts ${META_MTS_OPENWRT}" >> "${FEEDS_CONF}"
        log_success "Added meta-mts-openwrt to feeds.conf.default"
    fi
else
    echo "src-git mts ${META_MTS_OPENWRT}" > "${FEEDS_CONF}"
    log_success "Created feeds.conf.default with meta-mts-openwrt"
fi

# Step 7: Create target configurations
mkdir -p "${WORK_DIR}/configs"

# OLT GPON target
cat > "${WORK_DIR}/configs/olt2000.config" <<'OLT_EOF'
# MTS-OLT-2000 Target Configuration
# Target: armsr / armv10
# SoC: RTL960x + Tofino 2
CONFIG_TARGET_armsr=y
CONFIG_TARGET_armsr_armv10=y
CONFIG_TARGET_armsR_armv10_DEVICE_mts_olt2000=y

# Kernel version
CONFIG_KERNEL_VERSION_6_6=y
CONFIG_KERNEL_CONFIG="configs/olt2000-kernel.config"

# Required packages
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_luci-app-tr069=y
CONFIG_PACKAGE_omcc=y
CONFIG_PACKAGE_htop=y
CONFIG_PACKAGE_ip-full=y
CONFIG_PACKAGE_firewall4=y
CONFIG_PACKAGE_nftables=y
CONFIG_PACKAGE_dnsmasq-full=y
CONFIG_PACKAGE_dropbear=y
CONFIG_PACKAGE_sshpass=y
CONFIG_PACKAGE_usteer=y
CONFIG_PACKAGE_wifi-scripts=y
OLT_EOF
log_success "Created OLT-2000 target config"

# Enterprise target
cat > "${WORK_DIR}/configs/er1000.config" <<'ER_EOF'
# MTS-ER-1000 Target Configuration
# Target: freescale / armv8
# SoC: S32G3 + TomTom
CONFIG_TARGET_freescale=y
CONFIG_TARGET_freescale_armv8=y
CONFIG_TARGET_freescale_armv8_DEVICE_mts_er1000=y

# Kernel version
CONFIG_KERNEL_VERSION_6_6=y
CONFIG_KERNEL_CONFIG="configs/er1000-kernel.config"

# Required packages
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_sdwan=y
CONFIG_PACKAGE_frr=y
CONFIG_PACKAGE_bird2=y
CONFIG_PACKAGE_opkg=y
CONFIG_PACKAGE_ip-full=y
CONFIG_PACKAGE_firewall4=y
CONFIG_PACKAGE_nftables=y
CONFIG_PACKAGE_dnsmasq-full=y
CONFIG_PACKAGE_dropbear=y
CONFIG_PACKAGE_sshpass=y
CONFIG_PACKAGE_ipsec-vti-scripts=y
CONFIG_PACKAGE_libipsec=y
CONFIG_PACKAGE_luci-app-sdwan=y
ER_EOF
log_success "Created ER-1000 target config"

# Residential target
cat > "${WORK_DIR}/configs/rg500.config" <<'RG_EOF'
# MTS-RG-500 Target Configuration
# Target: mediatek / mt7981
# SoC: MT7981 + RTL960x
CONFIG_TARGET_mediatek=y
CONFIG_TARGET_mediatek_mt7981=y
CONFIG_TARGET_mediatek_mt7981_DEVICE_mts_rg500=y

# Kernel version
CONFIG_KERNEL_VERSION_6_6=y
CONFIG_KERNEL_CONFIG="configs/rg500-kernel.config"

# Required packages
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_luci-app-cwmp=y
CONFIG_PACKAGE_cwmp=y
CONFIG_PACKAGE_asterisk=y
CONFIG_PACKAGE_asterisk-core-sounds-en-gsm=y
CONFIG_PACKAGE_opus=y
CONFIG_PACKAGE_iptables-mod-iw=y
CONFIG_PACKAGE_kmod-mt7981-wo=y
CONFIG_PACKAGE_kmod-mt76-core=y
CONFIG_PACKAGE_kmod-mt76-wifi=y
CONFIG_PACKAGE_iptables-mod-nat6=y
CONFIG_PACKAGE_dnsmasq-full=y
CONFIG_PACKAGE_dropbear=y
CONFIG_PACKAGE_sshpass=y
CONFIG_PACKAGE_usteer=y
CONFIG_PACKAGE_wifi-scripts=y
CONFIG_PACKAGE_iptables-mod-tproxy=y
CONFIG_PACKAGE_iptables-mod-conntrack-extra=y
RG_EOF
log_success "Created RG-500 target config"

# Step 8: Display next steps
echo ""
echo "========================================================"
echo "  OpenWrt SDK Initialization Complete"
echo "========================================================"
echo ""
echo "Next steps:"
echo "  1. Source the environment: source ${SETUP_SCRIPT}"
echo "  2. Configure target:"
echo "     make defconfig"
echo "  3. Add custom packages:"
echo "     make menuconfig"
echo "  4. Build:"
echo "     make -j\$(nproc)"
echo ""
echo "Build artifacts will be in:"
echo "  ${OPENWRT_DIR}/bin/targets/"
echo ""
