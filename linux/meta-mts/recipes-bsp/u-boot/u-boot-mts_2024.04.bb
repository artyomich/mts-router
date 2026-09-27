# MTS U-Boot bootloader recipe
# Provides U-Boot for all MTS Router devices

SUMMARY = "MTS Router U-Boot bootloader"
DESCRIPTION = "Customized U-Boot bootloader for MTS Router devices with DPDK, PCIe, and networking support"

LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=b234ee4d69f5fce4486a80fdaf4a4213"

SRC_URI = "git://source.denx.de/u-boot/u-boot.git;branch=v2024.04;protocol=https \
           file://defconfig \
           file://uEnv.txt \
           "

SRCREV = "abc123def456789012345678901234567890abcd"

S = "${WORKDIR}/git"

inherit u-boot

# ============================================================
# Machine-specific defconfig
# ============================================================
def get_mts_defconfig(d):
    machine = d.getVar('MACHINE')
    configs = {
        'mts-cr9000': 'mts_cr9000_defconfig',
        'mts-mc5000': 'mts_mc5000_defconfig',
        'mts-mb3000': 'mts_mb3000_defconfig',
        'mts-olt2000': 'mts_olt2000_defconfig',
        'mts-er1000': 'mts_er1000_defconfig',
        'mts-rg500': 'mts_rg500_defconfig',
    }
    return configs.get(machine, 'mts_default_defconfig')

U_BOOT_DEFCONFIG = "${@get_mts_defconfig(d)}"

# ============================================================
# U-Boot configuration
# ============================================================
U_BOOT_CONFIG:mts-cr9000 = "mts_cr9000_defconfig,arch=arm"
U_BOOT_CONFIG:mts-mc5000 = "mts_mc5000_defconfig,arch=arm"
U_BOOT_CONFIG:mts-mb3000 = "mts_mb3000_defconfig,arch=arm"
U_BOOT_CONFIG:mts-olt2000 = "mts_olt2000_defconfig,arch=arm"
U_BOOT_CONFIG:mts-er1000 = "mts_er1000_defconfig,arch=arm"
U_BOOT_CONFIG:mts-rg500 = "mts_rg500_defconfig,arch=arm"

# ============================================================
# U-Boot environment
# ============================================================
do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 ${S}/u-boot.bin ${DEPLOYDIR}/u-boot-${MACHINE}.bin
    install -m 0644 ${WORKDIR}/uEnv.txt ${DEPLOYDIR}/uEnv-${MACHINE}.txt
    
    # Create FIT image if available
    if [ -f ${S}/u-boot.itb ]; then
        install -m 0644 ${S}/u-boot.itb ${DEPLOYDIR}/u-boot-${MACHINE}.itb
    fi
}

addtask deploy after do_compile before do_build

# ============================================================
# Machine-specific defconfig content
# ============================================================

# mts-cr9000 defconfig
# CONFIG_SYSTEM_PERF is set by defconfig

# ============================================================
# Install defconfigs
# ============================================================
do_install:append() {
    install -d ${D}${sysconfdir}/u-boot
    install -m 0644 ${WORKDIR}/defconfig ${D}${sysconfdir}/u-boot/defconfig-${MACHINE}
}

# ============================================================
# File dependencies
# ============================================================
FILES:${PN} = "${sysconfdir}/u-boot"
FILES:${PN}-dev = ""
FILES:${PN}-dbg = ""

# ============================================================
# Inherit classes
# ============================================================
inherit deploy

# ============================================================
# Machine compatibility
# ============================================================
COMPATIBLE_MACHINE = "(mts-cr9000|mts-mc5000|mts-mb3000|mts-olt2000|mts-er1000|mts-rg500)"
