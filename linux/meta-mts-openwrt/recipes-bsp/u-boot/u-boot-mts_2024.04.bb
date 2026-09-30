# U-Boot bootloader for MTS Router devices

SUMMARY = "U-Boot bootloader for MTS Router devices"
DESCRIPTION = "U-Boot bootloader customized for MTS Router hardware platforms."

LICENSE = "GPL-2.0+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=2dd1ef1b923735462179498594a4c1be"

SRC_URI = "git://github.com/u-boot/u-boot.git;branch=v2024.04;rev=v2024.04.0 \
           file://uEnv.txt \
           ${@'file://mts-${MACHINE}-defconfig' if 'mts-' in d.getVar('MACHINE', True) else ''}"

SRCREV = "abc123def456789"

S = "${WORKDIR}/git"

require recipes-bsp/u-boot/u-boot-common.inc

UBOOT_CONFIG_MACHINE = "mts-${MACHINE},mts-${MACHINE}_defconfig"
UBOOT_CONFIG ??= "${UBOOT_CONFIG_MACHINE}"

do_deploy:append() {
    # Deploy U-Boot images
    install -d ${DEPLOYDIR}/u-boot
    install -m 0644 ${B}/u-boot.bin ${DEPLOYDIR}/u-boot/
    install -m 0644 ${B}/u-boot.dtb ${DEPLOYDIR}/u-boot/
}

FILES:${PN} += "${sysconfdir}/u-boot/*"

INSANE_SKIP:${PN} += "staticdev"
