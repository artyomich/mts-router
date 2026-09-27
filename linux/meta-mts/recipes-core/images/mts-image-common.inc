# MTS Common Image Configuration
# Shared settings for all MTS Router device images

# ============================================================
# License
# ============================================================
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# ============================================================
# Machine compatibility
# ============================================================
COMPATIBLE_MACHINE = "(mts-cr9000|mts-mc5000|mts-mb3000|mts-olt2000|mts-er1000|mts-rg500)"

# ============================================================
# Package archives
# ============================================================
PACKAGE_CLASSES ?="package_rpm package_deb package-ipk"

# ============================================================
# SSTATE sharing
# ============================================================
SSTATE_MIRRORS ?= "file://.*;path=${SSTATE_MIRROR_PATH};downloadfilename=${SSTATE_MIRROR_URL}/\2;\2"

# ============================================================
# Default image features
# ============================================================
IMAGE_FEATURES:append = " ssh-server-opensysv package-management"

# ============================================================
# Default image fstypes
# ============================================================
IMAGE_FSTYPES:default = "ext4 wic.gz"

# ============================================================
# Image rootfs size defaults
# ============================================================
IMAGE_ROOTFS_SIZE:default = "4194304"
IMAGE_ROOTFS_MAXSIZE:default = "8388608"

# ============================================================
# Boot configuration
# ============================================================
IMAGE_BOOT_FILES:default = "boot/vmlinuz boot/initrd.img boot/extlinux/extlinux.conf"

# ============================================================
# MTS board class
# ============================================================
inherit mts-board

# ============================================================
# Core image
# ============================================================
inherit core-image

# ============================================================
# Rootfs
# ============================================================
inherit populate-rootfs

# ============================================================
# Deploy
# ============================================================
inherit deploy
