# MTS-CR-9000 Core Router Yocto Image
# Tofino 2 ASIC + AMD EPYC processor
# Full-featured embedded Linux with BGP/MPLS/SRv6 support

SUMMARY = "MTS-CR-9000 Core Router full image"
DESCRIPTION = "Complete Yocto image for MTS-CR-9000 Core Router with Tofino 2 ASIC support, BGP/MPLS/SRv6 stack, and DPDK high-performance forwarding"

require mts-image-common.inc

IMAGE_LINGUAS = " "

LICENSE = "MIT"

# ============================================================
# Image files
# ============================================================
IMAGE_FSTYPES = "ext4 wic.gz"
IMAGE_ROOTFS_SIZE = "8388608"
IMAGE_ROOTFS_MAXSIZE = "16777216"
IMAGE_BOOT_FILES = "boot/vmlinuz boot/extlinux/extlinux.conf"

# ============================================================
# Image features
# ============================================================
IMAGE_FEATURES += "\
    ssh-server-opensysv \
    package-management \
    cmdline-utils \
    sysfs-utils \
    dev-utils \
    tools-debug \
    tools-profile \
    dmesg \
    telemetry-dump \
    doc-utils \
    man-pages \
"

# ============================================================
# Package groups
# ============================================================
IMAGE_INSTALL:append = " \
    packagegroup-core-buildessential \
    packagegroup-core-devutils \
    packagegroup-core-full-cmdline \
"

# ============================================================
# MTS-specific packages
# ============================================================
IMAGE_INSTALL:append:mtscb9000 = " \
    mts-api-server \
    mts-sdk \
    mts-tofino2-driver \
    mts-tofino2-firmware \
    mts-tofino2-sde \
    dpdk \
    dpdk-app-pktgen \
    dpdk-app-testpmd \
    dpdk-devtools \
    frrouting \
    frrouting-bgpd \
    frrouting-ospfd \
    frrouting-ospf6d \
    frrouting-pimd \
    frrouting-ripd \
    frrouting-ripngd \
    frrouting-ldp \
    frrouting-bmp \
    frrouting-telemetryd \
    netopeer2-server \
    libyang \
    yang-tools \
    libnetconf2 \
    libssh \
    openssh \
    openssh-sftp-server \
    openssh-keygen \
    iproute2 \
    iproute2-sshtime \
    iproute2-nlmon \
    ipsec-tools \
    strongswan \
    strongswan-swanctl \
    strongswan-ipsecctl \
    strongswan-charon \
    strongswan-certgen \
    strongswan-eap-fast \
    strongswan-eap-identity \
    strongswan-eap-md5 \
    strongswan-eap-peap \
    strongswan-eap-tls \
    strongswan-eap-ttls \
    strongswan-mysql \
    strongswan-sqlite \
    strongswan-ldap \
    strongswan-openssl \
    net-tools \
    iputils \
    iputils-arping \
    iputils-ping \
    iputils-tracepath \
    iputils-traceroute \
    iperf3 \
    iperf \
    tcpdump \
    wireshark \
    nmap \
    mtr \
    bind-utils \
    curl \
    wget \
    jq \
    python3 \
    python3-core \
    python3-pip \
    python3-requests \
    python3-pyats \
    python3-scapy \
    python3-pandas \
    python3-numpy \
    prometheus-node-exporter \
    telegraf \
    influxdb \
    grafana \
    mts-telemetry-agent \
    mts-health-monitor \
    mts-firmware-manager \
    mts-config-agent \
    mts-ntp-client \
    mts-snmp-agent \
    mts-rsyslog \
    mts-logrotate \
    mts-syslog-ng \
    mts-chrony \
    mts-systemd \
    mts-systemd-boot \
    mts-sysvinit \
    mts-udev-rules \
    mts-fstab-generator \
    mts-hostnamed \
    mts-localed \
    mts-machined \
    mts-networkd \
    mts-resolved \
    mts-timesyncd \
    mts-journald \
    mts-udev \
    mts-sysctl \
    mts-modules-load \
    mts-random-seed \
    mts-userdb \
    mts-hwdb \
    mts-kbdmap \
    mts-locales \
    mts-timezone \
    mts-ntp \
    mts-dns \
    mts-hostname \
    mts-macmap \
    mts-machined \
    mts-network \
    mts-resolve \
    mts-timesync \
    mts-journal \
    mts-user \
    mts-group \
    mts-passwd \
    mts-shadow \
    mts-gshadow \
    mts-gshadow \
    mts-pam \
    mts-pam-modules \
    mts-pam-configs \
    mts-pam-extra \
    mts-pam-extra-access \
    mts-pam-extra-apparmor \
    mts-pam-extra-cracklib \
    mts-pam-extra-debuguid \
    mts-pam-extra-deny \
    mts-pam-extra-dummy \
    mts-pam-extra-faillog \
    mts-pam-extra-lastlog \
    mts-pam-extra-login \
    mts-pam-extra-mail \
    mts-pam-extra-motd \
    mts-pam-extra-nologin \
    mts-pam-extra-passwd \
    mts-pam-extra-pwquality \
    mts-pam-extra-recent \
    mts-pam-extra-securenetwork \
    mts-pam-extra-sepermit \
    mts-pam-extra-shelltime \
    mts-pam-extra-stack \
    mts-pam-extra-tmpfiles \
    mts-pam-extra-timestamp \
    mts-pam-extra-u2f \
    mts-pam-extra-utmp \
    mts-pam-extra-utmp-systemd \
    mts-pam-extra-verify \
    mts-pam-extra-xauth \
    mts-pam-extra-yubikey \
"

# ============================================================
# Image post-processing
# ============================================================
IMAGE_POSTPROCESS_COMMAND += " \
    mts-cr9000-rootfs-layout; \
    mts-cr9000-boot-config; \
"

# ============================================================
# Root filesystem layout
# ============================================================
mts-cr9000-rootfs-layout() {
    # Create MTS-specific directories
    install -d ${D}/etc/mts
    install -d ${D}/etc/mts/config
    install -d ${D}/etc/mts/certs
    install -d ${D}/etc/mts/firmware
    install -d ${D}/etc/mts/pipelines
    install -d ${D}/var/log/mts
    install -d ${D}/var/run/mts
    install -d ${D}/var/lib/mts
    install -d ${D}/var/lib/mts/frr
    install -d ${D}/var/lib/mts/bgp
    install -d ${D}/var/lib/mts/mpls
    install -d ${D}/var/lib/mts/p4runtime
    install -d ${D}/opt/mts
    install -d ${D}/opt/mts/bin
    install -d ${D}/opt/mts/lib
    install -d ${D}/opt/mts/share
    install -d ${D}/opt/mts/share/firmware
    install -d ${D}/opt/mts/share/pipelines
    install -d ${D}/opt/mts/share/config

    # Create MTS device nodes
    mknod -m 0666 ${D}/dev/mts0 c 240 0 2>/dev/null || true
    mknod -m 0666 ${D}/dev/mts1 c 240 1 2>/dev/null || true
}

# ============================================================
# Boot configuration
# ============================================================
mts-cr9000-boot-config() {
    # Create extlinux configuration
    install -d ${DEPLOY_DIR_IMAGE}/extlinux
    cat > ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf << EOF
LABEL mts-cr9000
    MENU LABEL MTS-CR-9000 Core Router
    KERNEL /boot/vmlinuz
    APPEND root=/dev/mmcblk0p2 rw rootwait console=tty0 console=ttyS0,115200n8 mts.board=cr9000 mts.os=yocto dpdk.hugepages=256 intel_iommu=on iommu=pt
    INITRD /boot/initrd.img
EOF

    # Copy to deploy directory
    cp ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf ${DEPLOY_DIR_IMAGE}/
}

# ============================================================
# Image deployment
# ============================================================
do_deploy:append() {
    # Create image name
    local img_name="mts-cr9000-image-${PV}.ext4"
    local img_gz_name="mts-cr9000-image-${PV}.ext4.gz"

    # Copy images to deploy directory
    cp ${DEPLOY_DIR_IMAGE}/images/mts-cr9000/${IMAGE_ROOTFS} ${DEPLOY_DIR_IMAGE}/${img_name}
    gzip -k ${DEPLOY_DIR_IMAGE}/${img_name}
    cp ${DEPLOY_DIR_IMAGE}/${img_gz_name} ${DEPLOY_DIR_IMAGE}/

    # Create checksums
    cd ${DEPLOY_DIR_IMAGE}
    sha256sum ${img_name} > ${img_name}.sha256
    sha256sum ${img_gz_name} > ${img_gz_name}.sha256

    # Create manifest
    find ${D} -type f | LC_ALL=C sort > ${DEPLOY_DIR_IMAGE}/mts-cr9000-manifest.txt
}

# ============================================================
# Image artifacts
# ============================================================
FILES:${IMAGE_NAME} = ""

# ============================================================
# Inherit image classes
# ============================================================
inherit core-image do-rootfs

# ============================================================
# Image creation
# ============================================================
CORE_IMAGE_BASE_INSTALL:remove = "${CORE_IMAGE_BASE_INSTALL}"
CORE_IMAGE_BASE_INSTALL:append = " ${CORE_IMAGE_BASE_INSTALL}"

# ============================================================
# Image type
# ============================================================
IMAGE_TYPE = "ext4"

# ============================================================
# Image name
# ============================================================
IMAGE_NAME = "mts-cr9000-image"

# ============================================================
# Image version
# ============================================================
PV = "1.0.0"

# ============================================================
# Image license
# ============================================================
LICENSE = "MIT"

# ============================================================
# Image timestamp
# ============================================================
SRCREV = "n/a"

# ============================================================
# Image source
# ============================================================
SRC_URI = ""

# ============================================================
# Image SSTATE
# ============================================================
SSTATE_DIR = "${TOPDIR}/sstate-cache"

# ============================================================
# Image cache
# ============================================================
TMPDIR = "${TOPDIR}/tmp"

# ============================================================
# Image deploy
# ============================================================
DEPLOY_DIR_IMAGE = "${TOPDIR}/deploy-images/${MACHINE}"

# ============================================================
# Image deploy package
# ============================================================
PACKAGE_ARCH = "${MACHINE_ARCH}"

# ============================================================
# Image compatibility
# ============================================================
COMPATIBLE_MACHINE = "^mts-cr9000$"

# ============================================================
# Image serial
# ============================================================
SERIALCON = "ttyS0"

# ============================================================
# Image console
# ============================================================
CONSOLE = "tty0 ttyS0"

# ============================================================
# Image machine
# ============================================================
MACHINE ??= "mts-cr9000"

# ============================================================
# Image distro
# ============================================================
DISTRO ??= "mts"

# ============================================================
# Image tune
# ============================================================
TUNE_FEATURES ??= "m64 popcnt cx16 ssse3 sse4_2 popcnt fxsr lzcnt movbe adx f16c fma bmi1 bmi2 avx2 aes pclmulqdq abm bmi3 rdseed"

# ============================================================
# Image arch
# ============================================================
TARGET_ARCH = "x86-64"

# ============================================================
# Image tune
# ============================================================
TUNE_PKGARCH = "x86-64-v2"

# ============================================================
# Image vendor
# ============================================================
META_MTS_VENDOR = "MTS Digital"

# ============================================================
# Image board
# ============================================================
META_MTS_BOARD = "cr9000"

# ============================================================
# Image model
# ============================================================
META_MTS_MODEL = "CR-9000"

# ============================================================
# Image series
# ============================================================
META_MTS_SERIES = "Core Router"

# ============================================================
# Image class
# ============================================================
inherit mts-board

# ============================================================
# Image image
# ============================================================
inherit core-image

# ============================================================
# Image rootfs
# ============================================================
inherit populate-rootfs

# ============================================================
# Image deploy
# ============================================================
inherit deploy

# ============================================================
# Image image types
# ============================================================
IMAGE_FSTYPES = "ext4 wic.gz"

# ============================================================
# Image rootfs size
# ============================================================
IMAGE_ROOTFS_SIZE = "8388608"

# ============================================================
# Image rootfs maxsize
# ============================================================
IMAGE_ROOTFS_MAXSIZE = "16777216"

# ============================================================
# Image boot files
# ============================================================
IMAGE_BOOT_FILES = "boot/vmlinuz boot/initrd.img boot/extlinux/extlinux.conf"

# ============================================================
# Image packages
# ============================================================
export IMAGE_INSTALL

# ============================================================
# Image features
# ============================================================
export IMAGE_FEATURES

# ============================================================
# Image extras
# ============================================================
export EXTRA_IMAGEFEATURES

# ============================================================
# Image commands
# ============================================================
export IMAGE_CMD

# ============================================================
# Image postprocess
# ============================================================
export IMAGE_POSTPROCESS_COMMAND

# ============================================================
# Image deploy command
# ============================================================
export IMAGE_DEPLOY_CMD

# ============================================================
# Image deploy dir
# ============================================================
export IMAGE_DEPLOY_DIR

# ============================================================
# Image deploy name
# ============================================================
export IMAGE_DEPLOY_NAME

# ============================================================
# Image deploy fstype
# ============================================================
export IMAGE_DEPLOY_FSTYPE

# ============================================================
# Image deploy manifest
# ============================================================
export IMAGE_DEPLOY_MANIFEST

# ============================================================
# Image deploy checksum
# ============================================================
export IMAGE_DEPLOY_CHECKSUM

# ============================================================
# Image deploy sha256
# ============================================================
export IMAGE_DEPLOY_SHA256

# ============================================================
# Image deploy sha512
# ============================================================
export IMAGE_DEPLOY_SHA512

# ============================================================
# Image deploy md5
# ============================================================
export IMAGE_DEPLOY_MD5

# ============================================================
# Image deploy sig
# ============================================================
export IMAGE_DEPLOY_SIG

# ============================================================
# Image deploy sigkey
# ============================================================
export IMAGE_DEPLOY_SIGKEY

# ============================================================
# Image deploy sigpath
# ============================================================
export IMAGE_DEPLOY_SIGPATH

# ============================================================
# Image deploy sigtype
# ============================================================
export IMAGE_DEPLOY_SIGTYPE

# ============================================================
# Image deploy sigver
# ============================================================
export IMAGE_DEPLOY_SIGVER

# ============================================================
# Image deploy sigdate
# ============================================================
export IMAGE_DEPLOY_SIGDATE

# ============================================================
# Image deploy sigtime
# ============================================================
export IMAGE_DEPLOY_SIGTIME

# ============================================================
# Image deploy siguser
# ============================================================
export IMAGE_DEPLOY_SIGUSER

# ============================================================
# Image deploy sigemail
# ============================================================
export IMAGE_DEPLOY_SIGEMAIL

# ============================================================
# Image deploy sigorg
# ============================================================
export IMAGE_DEPLOY_SIGORG

# ============================================================
# Image deploy sigloc
# ============================================================
export IMAGE_DEPLOY_SIGLOC

# ============================================================
# Image deploy sigcomment
# ============================================================
export IMAGE_DEPLOY_SIGCOMMENT
