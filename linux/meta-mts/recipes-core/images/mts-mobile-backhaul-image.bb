# MTS-MB-3000 Mobile Backhaul Buildroot Image
# NXP S32G3 ARM32 + 88Q5242 switch
# Buildroot-based with MPLS-TP and PTP

SUMMARY = "MTS-MB-3000 Mobile Backhaul image"
DESCRIPTION = "Complete Buildroot image for MTS-MB-3000 Mobile Backhaul with MPLS-TP forwarding, PTP grandmaster, and DPDK support"

require mts-image-common.inc

LICENSE = "MIT"

# ============================================================
# Buildroot configuration
# ============================================================
BUILDROOT_SUPPORT = "1"
BUILDROOT_VERSION = "2024.02"
BUILDROOT_DEFCONFIG = "board/mts/s32g3_defconfig"

# ============================================================
# Image files
# ============================================================
IMAGE_FSTYPES = "ext4.gz wic.gz"
IMAGE_ROOTFS_SIZE = "4194304"
IMAGE_ROOTFS_MAXSIZE = "8388608"

# ============================================================
# Image features
# ============================================================
IMAGE_FEATURES += "ssh-server-opensysv package-management cmdline-utils"

# ============================================================
# Base packages
# ============================================================
IMAGE_INSTALL:append = " \
    packagegroup-core-buildessential \
    packagegroup-core-devutils \
"

# ============================================================
# MTS-specific packages for Mobile Backhaul
# ============================================================
IMAGE_INSTALL:append = " \
    mts-api-server \
    mts-s32g3-driver \
    mts-s32g3-firmware \
    dpdk \
    dpdk-app-testpmd \
    mpls-tp-forwarder \
    mpls-tp-manager \
    linuxptp \
    linuxptp-ptp4l \
    linuxptp-phc2sys \
    linuxptp-pps-tools \
    linuxptp-ptp4l-man \
    linuxptp-phc2sys-man \
    frrouting \
    frrouting-bgpd \
    frrouting-ospfd \
    frrouting-ldp \
    frrouting-mpls \
    strongswan \
    net-tools \
    iputils \
    iperf3 \
    tcpdump \
    mtr \
    curl \
    jq \
    python3 \
    python3-core \
    mts-telemetry-agent \
    mts-health-monitor \
    mts-ntp-client \
    mts-chrony \
    mts-systemd \
    mts-rsyslog \
    mts-logrotate \
"

# ============================================================
# Image post-processing
# ============================================================
IMAGE_POSTPROCESS_COMMAND += " mts-mb3000-boot-config;"

# ============================================================
# Boot configuration for MB-3000
# ============================================================
mts-mb3000-boot-config() {
    install -d ${DEPLOY_DIR_IMAGE}/extlinux
    cat > ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf << EOF
LABEL mts-mb3000
    MENU LABEL MTS-MB-3000 Mobile Backhaul
    KERNEL /boot/vmlinuz
    APPEND root=/dev/mmcblk0p2 rw rootwait console=ttyS0,115200n8 mts.board=mb3000 mts.os=buildroot dpdk.hugepages=128 fsl_enetc.init_type=3
    INITRD /boot/initrd.img
EOF
    cp ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf ${DEPLOY_DIR_IMAGE}/
}

# ============================================================
# Image deployment
# ============================================================
do_deploy:append() {
    local img_name="mts-mb3000-image-${PV}.ext4.gz"

    cp ${DEPLOY_DIR_IMAGE}/images/mts-mb3000/${IMAGE_ROOTFS} ${DEPLOY_DIR_IMAGE}/${img_name}

    cd ${DEPLOY_DIR_IMAGE}
    sha256sum ${img_name} > ${img_name}.sha256

    find ${D} -type f | LC_ALL=C sort > ${DEPLOY_DIR_IMAGE}/mts-mb3000-manifest.txt
}

COMPATIBLE_MACHINE = "^mts-mb3000$"
IMAGE_NAME = "mts-mb3000-image"
PV = "1.0.0"
