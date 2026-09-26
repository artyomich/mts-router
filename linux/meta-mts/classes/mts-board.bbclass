# MTS Board Base Class
# Provides common board configuration for all MTS Router devices

# ============================================================
# Board-specific variables
# ============================================================

# Machine model name
MTS_BOARD_MODEL ?= "${MACHINE}"

# Board vendor
MTS_BOARD_VENDOR ?= "MTS Digital"

# Default kernel version
MTS_KERNEL_VERSION ?= "6.6"

# Default OS type
# yocto, buildroot, openwrt
MTS_BOARD_OS ?= "yocto"

# ============================================================
# Kernel configuration
# ============================================================

# Include MTS kernel configuration
KERNEL_EXTRA_ARGS = "devicetree-base = ${LAYERDIR}/recipes-kernel/linux/mts-dtbs-${MTS_BOARD_MODEL}.dtb"

# DPDK support
KERNEL_MTS_DPDK ?= "y"
KERNEL_MTS_MPLS ?= "y"
KERNEL_MTS_SR ?= "y"
KERNEL_MTS_PTP ?= "y"
KERNEL_MTS_SYNC_E ?= "y"

# ============================================================
# Package groups
# ============================================================

# Common packages for all MTS devices
MTS_COMMON_PACKAGES = "\
    mts-api \
    mts-sdk \
    dpdk \
    frrouting \
    net-tools \
    iproute2 \
    ipsec-tools \
"

# Core Router packages
MTS_CR_PACKAGES ?= "\
    mts-cr9000-firmware \
    mts-tofino2-driver \
    mts-telemetry-agent \
    prometheus-node-exporter \
"

# Mobile Core packages
MTS_MC_PACKAGES ?= "\
    mts-mc5000-firmware \
    mts-thunderx3-driver \
    k3s \
    containerd \
    cni-plugins \
"

# Mobile Backhaul packages
MTS_MB_PACKAGES ?= "\
    mts-mb3000-firmware \
    mts-s32g3-driver \
    linuxptp \
    mpls-tp-forwarder \
"

# OLT GPON packages
MTS_OLT_PACKAGES ?= "\
    mts-olt2000-firmware \
    mts-rtl960x-driver \
    cwmpd \
    omci-handler \
    luci \
"

# Enterprise Router packages
MTS_ER_PACKAGES ?= "\
    mts-er1000-firmware \
    mts-s32g3-driver \
    strongswan \
    sdwan-engine \
"

# Residential Gateway packages
MTS_RG_PACKAGES ?= "\
    mts-rg500-firmware \
    mts-mt7981-driver \
    asterisk \
    cwmpd \
    mt76-firmware \
    luci \
"

# ============================================================
# Image configuration
# ============================================================

# Root filesystem layout
ROOTFS_POSTPROCESS_COMMAND += "mts-rootfs-layout; "

mts-rootfs-layout() {
    # Create MTS-specific directories
    install -d ${D}/etc/mts
    install -d ${D}/etc/mts/config
    install -d ${D}/etc/mts/certs
    install -d ${D}/var/log/mts
    install -d ${D}/var/run/mts
}

# Boot configuration
MTS_BOOT_TYPE ?= "extlinux"
MTS_BOOT_MENU ?= "yes"

# ============================================================
# Firmware management
# ============================================================

# Firmware image name
MTS_FIRMWARE_IMAGE = "mts-${MTS_BOARD_MODEL}-firmware-${PV}.img"

# Firmware checksum algorithm
MTS_FIRMWARE_CHECKSUM_ALGO ?= "sha256"

# ============================================================
# Telemetry
# ============================================================

# Enable telemetry by default
MTS_TELEMETRY_ENABLED ?= "1"
MTS_TELEMETRY_INTERVAL ?= "10"
MTS_TELEMETRY_ENCODING ?= "protobuf"

# ============================================================
# HA configuration
# ============================================================

MTS_HA_ENABLED ?= "1"
MTS_HA_FAILOVER_TIME ?= "50"
MTS_HA_MODE ?= "active-standby"

# ============================================================
# Machine-specific overrides
# ============================================================

# Include machine-specific configuration
include ${LAYERDIR}/conf/machine/include/mts-${MTS_BOARD_MODEL}.inc
