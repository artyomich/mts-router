# Yocto Build Guide для MTS Router

## Обзор

Полная инфраструктура Yocto для сборки образов всех 6 устройств MTS Router.

## Структура meta-mts слоя

```
linux/meta-mts/
├── conf/
│   ├── layer.conf                    # Определение слоя
│   └── machine/
│       ├── mts-cr9000.conf           # Core Router machine
│       ├── mts-mc5000.conf           # Mobile Core machine
│       ├── mts-mb3000.conf           # Mobile Backhaul machine
│       ├── mts-olt2000.conf          # OLT GPON machine
│       ├── mts-er1000.conf           # Enterprise Router machine
│       └── mts-rg500.conf            # Residential Gateway machine
│       └── include/
│           └── mts-base.inc          # Базовые настройки
├── classes/
│   └── mts-board.bbclass             # Общий класс
├── recipes-bsp/
│   └── u-boot/
│       ├── u-boot-mts_2024.04.bb     # U-Boot recipe
│       └── files/
│           ├── uEnv.txt              # U-Boot env
│           ├── mts-cr9000_defconfig  # CR-9000 U-Boot config
│           ├── mts-mc5000_defconfig  # MC-5000 U-Boot config
│           ├── mts-mb3000_defconfig  # MB-3000 U-Boot config
│           ├── mts-olt2000_defconfig # OLT-2000 U-Boot config
│           ├── mts-er1000_defconfig  # ER-1000 U-Boot config
│           └── mts-rg500_defconfig   # RG-500 U-Boot config
├── recipes-core/
│   ├── images/
│   │   ├── mts-core-router-image.bb  # CR-9000 образ
│   │   ├── mts-mobile-core-image.bb  # MC-5000 образ
│   │   ├── mts-mobile-backhaul-image.bb # MB-3000 образ
│   │   ├── mts-olt-gpon-image.bb     # OLT-2000 образ
│   │   ├── mts-enterprise-image.bb   # ER-1000 образ
│   │   ├── mts-residential-image.bb  # RG-500 образ
│   │   ├── mts-image-common.inc      # Общие настройки
│   │   └── mts-image.bbappend        # Append
│   └── packages/
│       └── mts-drivers/
│           └── mts-drivers.bb        # Firmware packages
├── recipes-kernel/
│   ├── linux/
│   │   ├── mts-kernel_%.bbappend     # Kernel append
│   │   ├── mts-kernel-%25.bbappend   # Kernel append 2
│   │   ├── configs/
│   │   │   ├── mts-base.cfg          # Базовый kernel config
│   │   │   ├── mts-cr9000.cfg        # CR-9000 kernel config
│   │   │   ├── mts-mc5000.cfg        # MC-5000 kernel config
│   │   │   ├── mts-mb3000.cfg        # MB-3000 kernel config
│   │   │   ├── mts-olt2000.cfg       # OLT-2000 kernel config
│   │   │   ├── mts-er1000.cfg        # ER-1000 kernel config
│   │   │   └── mts-rg500.cfg         # RG-500 kernel config
│   │   └── files/
│   │       ├── device-tree/
│   │       │   ├── mts-cr9000.dts    # CR-9000 device tree
│   │       │   ├── mts-mc5000.dts    # MC-5000 device tree
│   │       │   ├── mts-mb3000.dts    # MB-3000 device tree
│   │       │   ├── mts-olt2000.dts   # OLT-2000 device tree
│   │       │   ├── mts-er1000.dts    # ER-1000 device tree
│   │       │   └── mts-rg500.dts     # RG-500 device tree
│   │       └── mts-kernel-configs/
│   │           └── common.cfg         # Common kernel config
│   └── dtc/
│       └── device-tree/
├── recipes-extended/
│   ├── cwmp/
│   │   └── cwmp-agent_1.0.bb         # TR-069 agent
│   ├── asterisk/
│   │   └── asterisk_20.bb            # VoIP
│   └── miniupnpd/
│       └── miniupnpd_2.2.bb          # UPnP
└── README.md
```

## Machine configurations

### mts-cr9000.conf (Core Router)

```bitbake
include conf/machine/include/mts-base.inc
MACHINE = "mts-cr9000"
SOC_FAMILY = "x86:tofino2"
KERNEL_DEVICETREE = "mts-cr9000.dtb"
UBOOT_MACHINE = "mts-cr9000_defconfig"
IMAGE_FSTYPES = "ext4.gz wic"
IMAGE_ROOTFS_SIZE = "131072"
SERIAL_CONSOLE = "115200 ttyS0"
MACHINE_FEATURES += "pcie nvme dpdk bgp mpls srv6"
```

### mts-mc5000.conf (Mobile Core)

```bitbake
include conf/machine/include/mts-base.inc
MACHINE = "mts-mc5000"
SOC_FAMILY = "arm64:thunderx3"
KERNEL_DEVICETREE = "mts-mc5000.dtb"
UBOOT_MACHINE = "mts-mc5000_defconfig"
IMAGE_FSTYPES = "ext4.gz wic"
IMAGE_ROOTFS_SIZE = "131072"
SERIAL_CONSOLE = "115200 ttyAMA0"
MACHINE_FEATURES += "arm64 dpdk k3s gtp-upf pfcp"
```

### mts-mb3000.conf (Mobile Backhaul)

```bitbake
include conf/machine/include/mts-base.inc
MACHINE = "mts-mb3000"
SOC_FAMILY = "arm:s32g3"
KERNEL_DEVICETREE = "mts-mb3000.dtb"
UBOOT_MACHINE = "mts-mb3000_defconfig"
IMAGE_FSTYPES = "ext4.gz tar.gz"
IMAGE_ROOTFS_SIZE = "65536"
SERIAL_CONSOLE = "115200 ttyS0"
MACHINE_FEATURES += "ptp synce mpls-tp"
```

### mts-olt2000.conf (OLT GPON)

```bitbake
include conf/machine/include/mts-base.inc
MACHINE = "mts-olt2000"
SOC_FAMILY = "x86:tofino2"
KERNEL_DEVICETREE = "mts-olt2000.dtb"
UBOOT_MACHINE = "mts-olt2000_defconfig"
IMAGE_FSTYPES = "ext4.gz squashfs"
IMAGE_ROOTFS_SIZE = "131072"
SERIAL_CONSOLE = "115200 ttyS0"
MACHINE_FEATURES += "gpon omci tr069"
```

### mts-er1000.conf (Enterprise Router)

```bitbake
include conf/machine/include/mts-base.inc
MACHINE = "mts-er1000"
SOC_FAMILY = "arm:s32g3"
KERNEL_DEVICETREE = "mts-er1000.dtb"
UBOOT_MACHINE = "mts-er1000_defconfig"
IMAGE_FSTYPES = "ext4.gz tar.gz"
IMAGE_ROOTFS_SIZE = "65536"
SERIAL_CONSOLE = "115200 ttyS0"
MACHINE_FEATURES += "sdwan mpls ipsec bgp"
```

### mts-rg500.conf (Residential Gateway)

```bitbake
include conf/machine/include/mts-base.inc
MACHINE = "mts-rg500"
SOC_FAMILY = "mtk:mt7981"
KERNEL_DEVICETREE = "mts-rg500.dtb"
UBOOT_MACHINE = "mts-rg500_defconfig"
IMAGE_FSTYPES = "ext4.gz squashfs"
IMAGE_ROOTFS_SIZE = "65536"
SERIAL_CONSOLE = "115200 ttyMT0"
MACHINE_FEATURES += "wifi gpon voip iptv usb"
```

## Kernel configs

### mts-cr9000.cfg (Core Router)

```bitbake
# Intel Tofino 2
CONFIG_INTEL_TOFINO2=y
CONFIG_P4RUNTIME=y
CONFIG_DPDK=y
CONFIG_DPDK_PMD_NETDEV=y
CONFIG_DPDK_PMD_PCAP=y

# PCIe/NVMe
CONFIG_PCI=y
CONFIG_PCIE_INTEL=y
CONFIG_BLK_DEV_NVME=y

# Networking
CONFIG_BGP=y
CONFIG_MPLS=y
CONFIG_SRH=y
CONFIG_IPV6_SEG6_HMAC=y
CONFIG_BRIDGE=y
CONFIG_VLAN_8021Q=y
CONFIG_IP_ADVANCED_ROUTER=y
CONFIG_IPV6=y

# Memory
CONFIG_HUGETLBFS=y
CONFIG_TRANSPARENT_HUGEPAGE=y
CONFIG_IOMMU_API=y
CONFIG_VFIO=y
CONFIG_VFIO_PCI=y
```

### mts-mc5000.cfg (Mobile Core)

```bitbake
# Marvell ThunderX3
CONFIG_ARM64=y
CONFIG_ARCH_THUNDER3=y
CONFIG_PCI=y
CONFIG_PCIE_MARVELL=y

# DPDK
CONFIG_DPDK=y
CONFIG_DPDK_PMD_NETDEV=y
CONFIG_HUGETLBFS=y
CONFIG_TRANSPARENT_HUGEPAGE=y

# 5G UPF
CONFIG_GTP=y
CONFIG_GTP_V2=y
CONFIG_IP_TUNNEL=y
CONFIG_VXLAN=y
CONFIG_GENEVE=y

# K3s/Kubernetes
CONFIG_CGROUPS=y
CONFIG_BLK_CGROUP=y
CONFIG_NETFILTER=y
CONFIG_NF_CONNTRACK=y
```

### mts-mb3000.cfg (Mobile Backhaul)

```bitbake
# NXP S32G3
CONFIG_ARCH_S32G=y
CONFIG_ARM=y
CONFIG_ARMV8=y

# DPDK
CONFIG_DPDK=y
CONFIG_HUGETLBFS=y
CONFIG_TRANSPARENT_HUGEPAGE=y

# PTP/1588
CONFIG_PTP_1588_CLOCK=y
CONFIG_PTP_1588_CLOCK_S32G=y
CONFIG_SYNC_E=y

# MPLS-TP
CONFIG_MPLS=y
CONFIG_MPLS_ROUTING=y
CONFIG_PSEUDO_TERMINAL=y
```

### mts-olt2000.cfg (OLT GPON)

```bitbake
# Intel Tofino 2
CONFIG_INTEL_TOFINO2=y
CONFIG_P4RUNTIME=y

# GPON/RTL960x
CONFIG_GPON=y
CONFIG_RTL960X=y
CONFIG_OMCI=y

# Networking
CONFIG_BRIDGE=y
CONFIG_VLAN_8021Q=y
CONFIG_IP_ADVANCED_ROUTER=y
CONFIG_IPV6=y
CONFIG_NETFILTER=y
```

### mts-er1000.cfg (Enterprise Router)

```bitbake
# NXP S32G3
CONFIG_ARCH_S32G=y
CONFIG_ARM=y

# TomTom ASIC
CONFIG_TOMTOM=y

# SD-WAN/IPsec
CONFIG_IPSEC=y
CONFIG_IP_VTI=y
CONFIG_IP_MROUTE=y
CONFIG_BGP=y
CONFIG_MPLS=y

# USB
CONFIG_USB=y
CONFIG_USB_EHCI_HCD=y
CONFIG_USB_OHCI_HCD=y
```

### mts-rg500.cfg (Residential Gateway)

```bitbake
# MediaTek MT7981
CONFIG_ARCH_MEDIATEK=y
CONFIG_MTK_SOC_MT7981=y
CONFIG_MTK_PPE=y
CONFIG_MTK_EPHY=y
CONFIG_MTK_ETH=y
CONFIG_MTK_WMAC=y

# WiFi 6
CONFIG_MAC80211=y
CONFIG_MT76_CORE=y
CONFIG_MT76x2E=y
CONFIG_MT7615E=y
CONFIG_MT76_CONNAC_FW=y

# GPON
CONFIG_GPON_RTL960X=y
CONFIG_OMCI=y

# VoIP
CONFIG_SND_SOC=y
CONFIG_SND_SOC_MT7981=y
CONFIG_PDM_DAC=y

# USB
CONFIG_USB=y
CONFIG_USB_EHCI_HCD=y
CONFIG_USB_OHCI_HCD=y

# TR-069
CONFIG_CWMP=y
```

## Image recipes

### mts-core-router-image.bb

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-cr9000-image"
IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-cr9000 \
    dpdk testpmd pktgen \
    frrouting bgpd ospfd ripd babeld \
    iproute2 iputils ethtool \
    strongswan ipsec-tools \
    iperf3 netperf \
    strace ltrace valgrind \
    procps psmisc \
"
```

### mts-mobile-core-image.bb

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-mc5000-image"
IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-mc5000 \
    dpdk testpmd \
    k3s-runtime k3s-extensions \
    gtpd pfcp-agent \
    iproute2 iputils ethtool \
    strongswan ipsec-tools \
    iperf3 netperf \
"
```

### mts-mobile-backhaul-image.bb

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-mb3000-image"
IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-mb3000 \
    dpdk testpmd \
    linuxptp ptp4l phc2sys \
    mpls-tp-tools \
    iproute2 iputils ethtool \
    iperf3 netperf \
"
```

### mts-olt-gpon-image.bb

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-olt2000-image"
IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-olt2000 \
    dpdk testpmd \
    cwmp-agent miniupnpd \
    iproute2 iputils ethtool \
    dnsmasq dhcp-server \
    iperf3 netperf \
"
```

### mts-enterprise-image.bb

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-er1000-image"
IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-er1000 \
    frrouting bgpd ospfd ripd \
    strongswan ipsec-tools \
    sdwan-agent \
    iproute2 iputils ethtool \
    iperf3 netperf \
"
```

### mts-residential-image.bb

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-rg500-image"
IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-rg500 \
    dnsmasq dhcp-server \
    strongswan ipsec-tools \
    asterisk libpri dahdi-tools \
    miniupnpd cwmp-agent \
    mt76-firmware \
    luci-base luci-app-firewall luci-app-dhcp \
    kmod-usb-core kmod-usb2 kmod-sound-mt7981 \
    netifd odhcpd odhcp6c ca-certificates \
    curl wget nano dropbear \
"
```

## Build process

### 1. Setup Yocto environment

```bash
# Clone Yocto/Poky
git clone -b kirkstone git://git.yoctoproject.org/poky.git
cd poky

# Init environment
source oe-init-build-env ../../mts-router/build/yocto

# Add meta-mts layer
bitbake-layers add-layer ../../mts-router/linux/meta-mts
bitbake-layers add-layer ../meta-openembedded/meta-oe
bitbake-layers add-layer ../meta-openembedded/meta-networking
bitbake-layers add-layer ../meta-virtualization
```

### 2. Configure target

```bash
# For Core Router
MACHINE=mts-cr9000 bitbake mts-core-router-image

# For Mobile Core
MACHINE=mts-mc5000 bitbake mts-mobile-core-image

# For Mobile Backhaul
MACHINE=mts-mb3000 bitbake mts-mobile-backhaul-image

# For OLT GPON
MACHINE=mts-olt2000 bitbake mts-olt-gpon-image

# For Enterprise Router
MACHINE=mts-er1000 bitbake mts-enterprise-image

# For Residential Gateway
MACHINE=mts-rg500 bitbake mts-residential-image
```

### 3. Build output

```
build/tmp/deploy/images/mts-cr9000/
├── mts-cr9000-image-*.ext4.gz
├── mts-cr9000-image-*.wic
├── zImage
├── mts-cr9000.dtb
├── u-boot.bin
├── uEnv.txt
└── sha256sum.txt
```

### 4. Verify image

```bash
# Check image checksums
cd build/tmp/deploy/images/mts-cr9000/
sha256sum -c sha256sum.txt

# Verify boot sequence
qemu-system-x86_64 -M q35 -cpu host \
    -kernel zImage \
    -initrd rootfs.cpio.gz \
    -append "console=ttyS0 root=/dev/ram" \
    -nographic \
    -m 4096 \
    -smp 4
```
