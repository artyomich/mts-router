# MTS-ER-1000 Enterprise Router OpenWrt Image
# NXP S32G3 ARM32 + TomTom switch
# OpenWrt-based with SD-WAN support

SUMMARY = "MTS-ER-1000 Enterprise Router OpenWrt image"
DESCRIPTION = "Complete OpenWrt image for MTS-ER-1000 Enterprise Router with SD-WAN engine, IPSec, and TomTom switch support"

require mts-image-common.inc

LICENSE = "MIT"

# ============================================================
# OpenWrt configuration
# ============================================================
OPENWRT_SUPPORT = "1"
OPENWRT_VERSION = "23.05"
OPENWRT_TARGET = "arm/armv7"
OPENWRT_SUBTARGET = "nxp_s32g"

# ============================================================
# Image files
# ============================================================
IMAGE_FSTYPES = "squashfs ext4 wic.gz"
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
    packagegroup-core-base-developer \
"

# ============================================================
# MTS-specific packages for Enterprise Router
# ============================================================
IMAGE_INSTALL:append = " \
    mts-api-server \
    mts-er1000-firmware \
    mts-s32g3-driver \
    mts-tomtom-driver \
    mts-tomtom-firmware \
    cwmpd \
    sdwan-engine \
    sdwan-manager \
    sdwan-path-selector \
    sdwan-policy-controller \
    sdwan-monitor \
    sdwan-health-check \
    sdwan-failover \
    sdwan-load-balancer \
    sdwan-app-detect \
    sdwan-traffic-shaper \
    sdwan-qos \
    sdwan-dscp \
    sdwan-marking \
    sdwan-routing \
    sdwan-bgp \
    sdwan-ospf \
    sdwan-static \
    sdwan-policy \
    sdwan-match \
    sdwan-action \
    sdwan-redirect \
    sdwan-nat \
    sdwan-dnat \
    sdwan-snat \
    sdwan-masq \
    strongswan \
    strongswan-swanctl \
    strongswan-charon \
    strongswan-ipsecctl \
    iproute2 \
    ipsec-tools \
    net-tools \
    iputils \
    tcpdump \
    curl \
    jq \
    python3 \
    mts-telemetry-agent \
    mts-health-monitor \
    mts-ntp-client \
    mts-chrony \
    mts-rsyslog \
    mts-logrotate \
"

# ============================================================
# Image post-processing
# ============================================================
IMAGE_POSTPROCESS_COMMAND += " mts-er1000-boot-config;"

# ============================================================
# Boot configuration for ER-1000
# ============================================================
mts-er1000-boot-config() {
    install -d ${DEPLOY_DIR_IMAGE}/extlinux
    cat > ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf << EOF
LABEL mts-er1000
    MENU LABEL MTS-ER-1000 Enterprise Router
    KERNEL /boot/vmlinuz
    APPEND root=/dev/mmcblk0p2 rw rootwait console=ttyS0,115200n8 mts.board=er1000 mts.os=openwrt fsl_enetc.init_type=3 iommu=pt
    INITRD /boot/initrd.img
EOF
    cp ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf ${DEPLOY_DIR_IMAGE}/
}

# ============================================================
# Image deployment
# ============================================================
do_deploy:append() {
    local img_name="mts-er1000-image-${PV}.squashfs"
    local img_gz_name="mts-er1000-image-${PV}.squashfs.gz"

    cp ${DEPLOY_DIR_IMAGE}/images/mts-er1000/${IMAGE_ROOTFS} ${DEPLOY_DIR_IMAGE}/${img_name}
    gzip -k ${DEPLOY_DIR_IMAGE}/${img_name}
    cp ${DEPLOY_DIR_IMAGE}/${img_gz_name} ${DEPLOY_DIR_IMAGE}/

    cd ${DEPLOY_DIR_IMAGE}
    sha256sum ${img_name} > ${img_name}.sha256
    sha256sum ${img_gz_name} > ${img_gz_name}.sha256

    find ${D} -type f | LC_ALL=C sort > ${DEPLOY_DIR_IMAGE}/mts-er1000-manifest.txt
}

COMPATIBLE_MACHINE = "^mts-er1000$"
IMAGE_NAME = "mts-er1000-image"
PV = "1.0.0"
