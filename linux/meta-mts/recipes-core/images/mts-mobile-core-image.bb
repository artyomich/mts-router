# MTS-MC-5000 Mobile Core Yocto Image
# ThunderX3 ARM64 + AMD EPYC
# Yocto + K3s (Kubernetes)

SUMMARY = "MTS-MC-5000 Mobile Core image with K3s"
DESCRIPTION = "Complete Yocto image for MTS-MC-5000 Mobile Core with 5G UPF/SMF/AMF/PCF, K3s container runtime, and DPDK support"

require mts-image-common.inc

LICENSE = "MIT"

# ============================================================
# Image files
# ============================================================
IMAGE_FSTYPES = "ext4 wic.gz"
IMAGE_ROOTFS_SIZE = "10485760"
IMAGE_ROOTFS_MAXSIZE = "20971520"

# ============================================================
# Image features
# ============================================================
IMAGE_FEATURES += "\
    ssh-server-opensysv \
    package-management \
    cmdline-utils \
    dev-utils \
    tools-debug \
    tools-profile \
    dmesg \
    telemetry-dump \
"

# ============================================================
# Base packages
# ============================================================
IMAGE_INSTALL:append = " \
    packagegroup-core-buildessential \
    packagegroup-core-devutils \
    packagegroup-core-full-cmdline \
"

# ============================================================
# MTS-specific packages for Mobile Core
# ============================================================
IMAGE_INSTALL:append = " \
    mts-api-server \
    mts-sdk \
    mts-thunderx3-driver \
    mts-thunderx3-firmware \
    dpdk \
    dpdk-app-pktgen \
    dpdk-app-testpmd \
    k3s \
    k3s-server \
    k3s-agent \
    k3s-core \
    k3s-cni \
    k3s-cni-plugins \
    containerd \
    containerd-runc \
    runc \
    crictl \
    kubectl \
    kubelet \
    kubeadm \
    calico \
    calico-cni \
    calico-node \
    calico-kube-controllers \
    calico-policy-controller \
    calico-felix \
    calico-typha \
    calico-flexvol \
    flannel \
    calicoctl \
    cilium \
    cilium-cli \
    cilium-operator \
    prometheus-node-exporter \
    telegraf \
    mts-telemetry-agent \
    mts-health-monitor \
    mts-firmware-manager \
    mts-config-agent \
    mts-ntp-client \
    mts-snmp-agent \
    mts-rsyslog \
    mts-logrotate \
    mts-chrony \
    mts-systemd \
    frrouting \
    frrouting-bgpd \
    frrouting-ospfd \
    frrouting-ospf6d \
    frrouting-pimd \
    netopeer2-server \
    libyang \
    yang-tools \
    libnetconf2 \
    libssh \
    openssh \
    openssh-sftp-server \
    iproute2 \
    ipsec-tools \
    strongswan \
    net-tools \
    iputils \
    iperf3 \
    tcpdump \
    nmap \
    mtr \
    bind-utils \
    curl \
    jq \
    python3 \
    python3-core \
    python3-pip \
    python3-requests \
    python3-scapy \
"

# ============================================================
# Image post-processing
# ============================================================
IMAGE_POSTPROCESS_COMMAND += " mts-mc5000-boot-config;"

# ============================================================
# Boot configuration for MC-5000
# ============================================================
mts-mc5000-boot-config() {
    install -d ${DEPLOY_DIR_IMAGE}/extlinux
    cat > ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf << EOF
LABEL mts-mc5000
    MENU LABEL MTS-MC-5000 Mobile Core
    KERNEL /boot/vmlinuz
    APPEND root=/dev/mmcblk0p2 rw rootwait console=tty0 console=ttyAMA0,115200n8 mts.board=mc5000 mts.os=yocto dpdk.hugepages=512 arm64.pamu=on iommu=pt
    INITRD /boot/initrd.img
EOF
    cp ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf ${DEPLOY_DIR_IMAGE}/
}

# ============================================================
# Image deployment
# ============================================================
do_deploy:append() {
    local img_name="mts-mc5000-image-${PV}.ext4"
    local img_gz_name="mts-mc5000-image-${PV}.ext4.gz"

    cp ${DEPLOY_DIR_IMAGE}/images/mts-mc5000/${IMAGE_ROOTFS} ${DEPLOY_DIR_IMAGE}/${img_name}
    gzip -k ${DEPLOY_DIR_IMAGE}/${img_name}
    cp ${DEPLOY_DIR_IMAGE}/${img_gz_name} ${DEPLOY_DIR_IMAGE}/

    cd ${DEPLOY_DIR_IMAGE}
    sha256sum ${img_name} > ${img_name}.sha256
    sha256sum ${img_gz_name} > ${img_gz_name}.sha256

    find ${D} -type f | LC_ALL=C sort > ${DEPLOY_DIR_IMAGE}/mts-mc5000-manifest.txt
}

COMPATIBLE_MACHINE = "^mts-mc5000$"
IMAGE_NAME = "mts-mc5000-image"
PV = "1.0.0"
