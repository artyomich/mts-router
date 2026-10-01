# MTS Kernel bbappend for all devices
# Provides device-specific kernel patches and configurations

# Kernel version
KERNEL_DEVICETREE = ""

# Include device tree source
DTB_${MACHINE} = "mts-${MACHINE}.dtb"

# Kernel patches location
SRC_URI:append:mts-cr9000 = " \
    file://0001-tofino2-p4-pipeline-driver.patch \
    file://0002-tofino2-phy-management.patch \
    file://0003-tofino2-telemetry-driver.patch \
    file://0004-amd-pcie-tuning.patch \
"

SRC_URI:append:mts-mc5000 = " \
    file://0001-thunderx3-net-driver.patch \
    file://0002-thunderx3-pmu-driver.patch \
    file://0003-thunderx3-cxl-driver.patch \
    file://0004-amd-pcie-tuning.patch \
"

SRC_URI:append:mts-mb3000 = " \
    file://0001-s32g3-net-driver.patch \
    file://0002-s32g3-sec-driver.patch \
    file://0003-s32g3-ptp-driver.patch \
    file://0004-s32g3-sync-driver.patch \
"

SRC_URI:append:mts-olt2000 = " \
    file://0001-tofino2-p4-pipeline-driver.patch \
    file://0002-rtl960x-gpon-driver.patch \
    file://0003-rtl960x-omci-driver.patch \
    file://0004-rtl960x-wdm-driver.patch \
"

SRC_URI:append:mts-er1000 = " \
    file://0001-s32g3-net-driver.patch \
    file://0002-s32g3-sec-driver.patch \
    file://0003-tomtom-asic-driver.patch \
    file://0004-tomtom-phy-driver.patch \
"

SRC_URI:append:mts-rg500 = " \
    file://0001-mt7981-net-driver.patch \
    file://0002-mt7981-wifi-driver.patch \
    file://0003-mt7981-crypto-driver.patch \
    file://0004-rtl960x-gpon-driver.patch \
"

# Kernel configuration fragments
KERNEL_EXTRA_FRAGMENTS = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-config-base.cfg \
    ${LAYERDIR}/recipes-kernel/linux/mts-dpdk.cfg \
    ${LAYERDIR}/recipes-kernel/linux/mts-mpls.cfg \
    ${LAYERDIR}/recipes-kernel/linux/mts-telemetry.cfg \
"

# Machine-specific configuration fragments
KERNEL_CONFIG_FRAGMENTS:append:mts-cr9000 = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-tofino2.cfg \
"

KERNEL_CONFIG_FRAGMENTS:append:mts-mc5000 = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-thunderx3.cfg \
"

KERNEL_CONFIG_FRAGMENTS:append:mts-mb3000 = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-s32g3.cfg \
"

KERNEL_CONFIG_FRAGMENTS:append:mts-olt2000 = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-tofino2.cfg \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-rtl960x.cfg \
"

KERNEL_CONFIG_FRAGMENTS:append:mts-er1000 = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-s32g3.cfg \
"

KERNEL_CONFIG_FRAGMENTS:append:mts-rg500 = " \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-mt7981.cfg \
    ${LAYERDIR}/recipes-kernel/linux/mts-kernel-rtl960x.cfg \
"

# Kernel compile options
KERNEL_LINK_FILENAME = "zImage"
