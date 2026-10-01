# MTS Kernel bbappend for all devices
# Provides device-specific kernel configuration and patches

# ============================================================
# Kernel version preference
# ============================================================

# Core Router / OLT GPON (x86_64) - Linux 6.6 LTS
PV_mts-kernel_mts-cr9000 = "6.6.45"
PV_mts-kernel_mts-olt2000 = "6.6.45"

# Mobile Core (ARM64) - Linux 6.6 LTS
PV_mts-kernel_mts-mc5000 = "6.6.45"

# Mobile Backhaul / Enterprise (ARM32) - Linux 6.1 LTS
PV_mts-kernel_mts-mb3000 = "6.1.85"
PV_mts-kernel_mts-er1000 = "6.1.85"

# Residential Gateway (ARM64) - Linux 6.1 LTS
PV_mts-kernel_mts-rg500 = "6.1.85"

# ============================================================
# Kernel configuration fragments
# ============================================================

# Include MTS kernel config fragment
KERNEL_EXTRA_BASELAYERS = "${LAYERDIR}/recipes-kernel/linux/configs"

KERNEL_CONFIG_FRAGMENTS += "${LAYERDIR}/recipes-kernel/linux/configs/mts-base.cfg"

# Device-specific kernel config fragments
KERNEL_CONFIG_FRAGMENTS_mts-cr9000 = "\
    ${LAYERDIR}/recipes-kernel/linux/configs/mts-cr9000.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/dpdk.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/p4runtime.cfg \
"

KERNEL_CONFIG_FRAGMENTS_mts-mc5000 = "\
    ${LAYERDIR}/recipes-kernel/linux/configs/mts-mc5000.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/dpdk.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/k3s.cfg \
"

KERNEL_CONFIG_FRAGMENTS_mts-mb3000 = "\
    ${LAYERDIR}/recipes-kernel/linux/configs/mts-mb3000.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/dpdk.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/ptp.cfg \
"

KERNEL_CONFIG_FRAGMENTS_mts-olt2000 = "\
    ${LAYERDIR}/recipes-kernel/linux/configs/mts-olt2000.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/p4runtime.cfg \
"

KERNEL_CONFIG_FRAGMENTS_mts-er1000 = "\
    ${LAYERDIR}/recipes-kernel/linux/configs/mts-er1000.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/mpls.cfg \
"

KERNEL_CONFIG_FRAGMENTS_mts-rg500 = "\
    ${LAYERDIR}/recipes-kernel/linux/configs/mts-rg500.cfg \
    ${LAYERDIR}/recipes-kernel/linux/configs/mt76.cfg \
"

# ============================================================
# Kernel patches
# ============================================================

# Core Router patches (Tofino 2)
SRC_URI_mts-cr9000 += "\
    file://0001-tofino2-pcie-driver.patch \
    file://0002-tofino2-p4-programming.patch \
    file://0003-x86-64-network-optimizations.patch \
"

# Mobile Core patches (ThunderX3)
SRC_URI_mts-mc5000 += "\
    file://0001-thunderx3-npa-pfdrv.patch \
    file://0002-arm64-network-optimizations.patch \
    file://0003-k3s-containerd-support.patch \
"

# Mobile Backhaul patches (S32G3)
SRC_URI_mts-mb3000 += "\
    file://0001-s32g3-fman-driver.patch \
    file://0002-arm-dpdk-optimizations.patch \
    file://0003-ptp-grandmaster-support.patch \
"

# OLT GPON patches (Tofino 2 + RTL960x)
SRC_URI_mts-olt2000 += "\
    file://0001-tofino2-pcie-driver.patch \
    file://0002-rtl960x-gpon-driver.patch \
    file://0003-openwrt-kernel-optimizations.patch \
"

# Enterprise Router patches (S32G3 + TomTom)
SRC_URI_mts-er1000 += "\
    file://0001-s32g3-fman-driver.patch \
    file://0002-tomtom-switch-driver.patch \
    file://0003-sdwan-kernel-support.patch \
"

# Residential Gateway patches (MT7981 + RTL960x)
SRC_URI_mts-rg500 += "\
    file://0001-mt7981-network-driver.patch \
    file://0002-rtl960x-gpon-driver.patch \
    file://0003-mt76-wifi-driver.patch \
    file://0004-voip-audio-support.patch \
"

# ============================================================
# Device tree sources
# ============================================================

# Core Router device tree
SRC_URI:append:mts-cr9000 = "file://device-tree/mts-cr9000.dts;subdir=git"

# Mobile Core device tree
SRC_URI:append:mts-mc5000 = "file://device-tree/mts-mc5000.dts;subdir=git"

# Mobile Backhaul device tree
SRC_URI:append:mts-mb3000 = "file://device-tree/mts-mb3000.dts;subdir=git"

# OLT GPON device tree
SRC_URI:append:mts-olt2000 = "file://device-tree/mts-olt2000.dts;subdir=git"

# Enterprise Router device tree
SRC_URI:append:mts-er1000 = "file://device-tree/mts-er1000.dts;subdir=git"

# Residential Gateway device tree
SRC_URI:append:mts-rg500 = "file://device-tree/mts-rg500.dts;subdir=git"

# ============================================================
# Kernel compile options
# ============================================================

# Enable debug info for all devices
KERNEL_DEBUG_FLAGS = "-g"

# Enable networking debug (disabled in production)
# KERNEL_DEBUG_KERNEL = "1"
# KERNEL_DEBUG_NET = "1"

# Performance optimizations
KERNEL_OPTIMIZATIONS = "-O2"

# Module loading
AUTO_KERNEL_MODULE_INSTALL = "1"
KERNEL_MODULE_AUTOLOAD = "1"
