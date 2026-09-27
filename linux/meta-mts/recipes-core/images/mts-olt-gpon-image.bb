# MTS-OLT-2000 OLT GPON OpenWrt Image
# Tofino 2 ASIC + AMD EPYC
# OpenWrt-based with GPON OMCI support

SUMMARY = "MTS-OLT-2000 OLT GPON OpenWrt image"
DESCRIPTION = "Complete OpenWrt image for MTS-OLT-2000 OLT GPON with Tofino 2 ASIC, RTL960x GPON support, TR-069, and OMCI management"

require mts-image-common.inc

LICENSE = "MIT"

# ============================================================
# OpenWrt configuration
# ============================================================
OPENWRT_SUPPORT = "1"
OPENWRT_VERSION = "23.05"
OPENWRT_TARGET = "x86/64"
OPENWRT_SUBTARGET = "generic"

# ============================================================
# Image files
# ============================================================
IMAGE_FSTYPES = "squashfs ext4 wic.gz"
IMAGE_ROOTFS_SIZE = "6291456"
IMAGE_ROOTFS_MAXSIZE = "12582912"

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
# MTS-specific packages for OLT GPON
# ============================================================
IMAGE_INSTALL:append = " \
    mts-api-server \
    mts-olt2000-firmware \
    mts-tofino2-driver \
    mts-rtl960x-driver \
    mts-rtl960x-firmware \
    cwmpd \
    cwmpd-utils \
    omci-handler \
    omci-manager \
    omci-gateway \
    omci-bridge \
    omci-ont \
    omci-onu \
    omci-ont-mgmt \
    omci-ont-provision \
    omci-ont-monitor \
    omci-ont-log \
    omci-ont-firmware \
    omci-ont-config \
    omci-ont-diagnostic \
    omci-ont-performance \
    omci-ont-alarms \
    omci-ont-events \
    omci-ont-logs \
    omci-ont-firmware-upgrade \
    omci-ont-config-backup \
    omci-ont-config-restore \
    omci-ont-diagnostic-run \
    omci-ont-performance-collect \
    omci-ont-alarms-clear \
    omci-ont-events-list \
    omci-ont-logs-list \
    luci \
    luci-base \
    luci-compat \
    luci-ssl \
    luci-proto-ipv6 \
    luci-proto-ppp \
    luci-proto-relay \
    luci-app-cwmp \
    luci-app-omci \
    luci-app-status \
    luci-app-system \
    luci-app-network \
    luci-app-firewall \
    luci-app-ddns \
    luci-app-upnp \
    luci-app-samba \
    luci-app-usb \
    luci-app-htop \
    luci-app-smartdns \
    luci-app-adguardmanager \
    luci-app-mwan3 \
    luci-app-mwan3helper \
    luci-app-opkg \
    luci-app-ramfree \
    luci-app-shadowsocks \
    luci-app-zerotier \
    kmod-usb-core \
    kmod-usb-net \
    kmod-usb-eth \
    kmod-usb-storage \
    kmod-usb2 \
    kmod-usb3 \
    kmod-mii \
    kmod-phy-realtek \
    kmod-phy-aquantia \
    kmod-tg3 \
    kmod-e1000e \
    kmod-ixgbe \
    kmod-i40e \
    kmod-iavf \
    kmod-ice \
    kmod-bnx2x \
    kmod-qlcnic \
    kmod-hinic \
    kmod-mlx4-core \
    kmod-mlx5-core \
    kmod-nfp-core \
    iproute2 \
    ipsec-tools \
    strongswan \
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
IMAGE_POSTPROCESS_COMMAND += " mts-olt2000-boot-config;"

# ============================================================
# Boot configuration for OLT-2000
# ============================================================
mts-olt2000-boot-config() {
    install -d ${DEPLOY_DIR_IMAGE}/extlinux
    cat > ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf << EOF
LABEL mts-olt2000
    MENU LABEL MTS-OLT-2000 OLT GPON
    KERNEL /boot/vmlinuz
    APPEND root=/dev/mmcblk0p2 rw rootwait console=tty0 console=ttyS0,115200n8 mts.board=olt2000 mts.os=openwrt intel_iommu=on iommu=pt
    INITRD /boot/initrd.img
EOF
    cp ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf ${DEPLOY_DIR_IMAGE}/
}

# ============================================================
# Image deployment
# ============================================================
do_deploy:append() {
    local img_name="mts-olt2000-image-${PV}.squashfs"
    local img_gz_name="mts-olt2000-image-${PV}.squashfs.gz"

    cp ${DEPLOY_DIR_IMAGE}/images/mts-olt2000/${IMAGE_ROOTFS} ${DEPLOY_DIR_IMAGE}/${img_name}
    gzip -k ${DEPLOY_DIR_IMAGE}/${img_name}
    cp ${DEPLOY_DIR_IMAGE}/${img_gz_name} ${DEPLOY_DIR_IMAGE}/

    cd ${DEPLOY_DIR_IMAGE}
    sha256sum ${img_name} > ${img_name}.sha256
    sha256sum ${img_gz_name} > ${img_gz_name}.sha256

    find ${D} -type f | LC_ALL=C sort > ${DEPLOY_DIR_IMAGE}/mts-olt2000-manifest.txt
}

COMPATIBLE_MACHINE = "^mts-olt2000$"
IMAGE_NAME = "mts-olt2000-image"
PV = "1.0.0"
