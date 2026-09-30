# meta-mts-openwrt — OpenWrt Layer для MTS Residential Gateways

## Структура слоя

```
meta-mts-openwrt/
├── conf/
│   ├── layer.conf                    # Определение слоя
│   └── mts-targets.conf              # Целевые платформы
├── conf/machine/
│   └── mts-rg500.conf                # Machine config RG-500
├── recipes-core/
│   └── images/
│       ├── mts-residential-image.bb  # Образ для RG-500
│       └── mts-image-common.inc      # Общие настройки
├── recipes-bsp/
│   └── u-boot/
│       ├── u-boot-mts_2024.04.bb     # U-Boot для RG-500
│       └── files/
│           └── mts-rg500_defconfig   # Конфиг U-Boot
├── recipes-kernel/
│   ├── linux/
│   │   ├── mts-kernel_%.bbappend     # Ядро с патчами
│   │   └── configs/
│   │       ├── mts-base.cfg          # Базовые настройки
│   │       └── mts-rg500.cfg         # Настройки для RG-500
│   └── dtc/
│       └── device-tree/
│           └── mts-rg500.dts         # Device tree
├── recipes-extended/
│   ├── cwmp/cwmp-agent_1.0.bb        # TR-069 агент
│   ├── asterisk/asterisk_20.bb       # VoIP (Asterisk)
│   └── miniupnpd/miniupnpd_2.2.bb    # UPnP
├── recipes-ml/
│   └── mt76-firmware/mt76-firmware_2024.bb
├── package/mt/mts-gateway/Makefile   # OpenWrt пакет
└── classes/mts-board.bbclass         # Базовый класс
```

## Конфигурация слоя

### conf/layer.conf

```bitbake
LAYERSERIES_COMPAT_meta-mts-openwrt = "kirkstone morty"
BBPATH .= ":${LAYERDIR}"
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb ${LAYERDIR}/recipes-*/*/*.bbappend"
LAYERDEPENDS_meta-mts-openwrt = "core openwrt"
LAYERSERIES_COMPAT_openwrt = "kirkstone morty"
```

### conf/mts-targets.conf

```bitbake
MTS_TARGETS = "mts-rg500"
MTS_IMAGE_TYPES = "ext4 squashfs"
MTS_KERNEL_VERSION = "6.6"
MTS_OPENWRT_TARGET = "mediatek"
MTS_OPENWRT_SUBTARGET = "filogic830"
```

## Machine configuration

### conf/machine/mts-rg500.conf

```bitbake
include conf/machine/include/mts-base.inc

MACHINE = "mts-rg500"
SOC_FAMILY = "mtk:mt7981"
KERNEL_DEVICETREE = "mediatek/mts-rg500.dtb"
UBOOT_MACHINE = "mts-rg500_defconfig"
IMAGE_FSTYPES = "ext4.gz tar.gz"
IMAGE_ROOTFS_SIZE = "65536"
SERIAL_CONSOLE = "115200 /dev/ttyMT0"
MACHINE_FEATURES += "wifi gpon voip iptv usb"

IMAGE_INSTALL:append = " \
    dnsmasq strongswan asterisk miniupnpd cwmp-agent mt76-firmware luci \
"
```

## Kernel config (mts-rg500.cfg)

```bitbake
# MediaTek MT7981
CONFIG_ARCH_MEDIATEK=y
CONFIG_MTK_SOC_MT7981=y
CONFIG_MTK_PPE=y
CONFIG_MTK_EPHY=y
CONFIG_MTK_ETH=y
CONFIG_MTK_WMAC=y

# GPON
CONFIG_GPON_RTL960X=y
CONFIG_OMCI=y
CONFIG_GPON_WDM=y

# VoIP
CONFIG_SND_SOC=y
CONFIG_SND_SOC_MT7981=y
CONFIG_SND_SOC_AIC3254=y
CONFIG_PDM_DAC=y

# WiFi 6
CONFIG_MAC80211=y
CONFIG_MT76_CORE=y
CONFIG_MT76x2E=y
CONFIG_MT7615E=y
CONFIG_MT76_CONNAC_FW=y

# Network
CONFIG_BRIDGE=y
CONFIG_VLAN_8021Q=y
CONFIG_IP_ADVANCED_ROUTER=y
CONFIG_IPV6=y

# IPsec
CONFIG_CRYPTO_AEAD=y
CONFIG_CRYPTO_AES=y
CONFIG_CRYPTO_SHA256=y
CONFIG_XFRM=y

# TR-069
CONFIG_CWMP=y

# Filesystem
CONFIG_EXT4_FS=y
CONFIG_SQUASHFS=y
CONFIG_MTD=y
```

## Device Tree (mts-rg500.dts)

```dts
/dts-v1/;
/ {
    model = "MTS Residential Gateway RG-500";
    compatible = "mts,mts-rg500", "mediatek,mt7981";

    aliases {
        serial0 = &uart0;
        eth0 = &gmac0;
        eth1 = &gmac1;
        wlan0 = &wifi0;
        wlan1 = &wifi1;
    };

    memory {
        reg = <0x0 0x40000000 0 0x10000000>;
    };

    leds {
        compatible = "gpio-leds";
        sys_led: sys { label = "mts-rg500:amber:sys"; gpios = <&gpio 12 GPIO_ACTIVE_LOW>; };
        pon_led: pon { label = "mts-rg500:green:pon"; gpios = <&gpio 15 GPIO_ACTIVE_LOW>; };
    };

    buttons {
        compatible = "gpio-keys";
        reset_btn: reset { label = "Reset"; gpios = <&gpio 20 GPIO_ACTIVE_LOW>; linux,code = <KEY_RESTART>; };
    };
};

&uart0 { status = "okay"; };
&eth { gmac0: mac@0 { compatible = "mediatek,eth-mac"; reg = <0>; phy-mode = "rgmii"; }; gmac1: mac@1 { compatible = "mediatek,eth-mac"; reg = <1>; phy-mode = "rgmii"; }; };
&ethsw { status = "okay"; mediatek,ethsw; };
&wmac { status = "okay"; mediatek,mtd-eeprom = <&factory 0x0>; };
&pci { status = "okay"; };
&wifi { status = "okay"; mediatek,mtd-eeprom = <&factory 0x8000>; };
&gp { gp0 { status = "okay"; rtk,pon-ont; }; };
```

## Image recipe (mts-residential-image.bb)

```bitbake
require mts-image-common.inc
IMAGE_NAME = "mts-rg500-image"

IMAGE_INSTALL:append = " \
    kernel-image uboot-mts dtb-mts-rg500 \
    busybox dropbear iproute2 iputils ethtool \
    dnsmasq dhcp-server strongswan ipsec-tools \
    asterisk libpri dahdi-tools miniupnpd cwmp-agent \
    mt76-firmware luci-base luci-app-firewall luci-app-dhcp \
    kmod-usb-core kmod-usb2 kmod-sound-mt7981 \
    netifd odhcpd odhcp6c ca-certificates openssl-util curl wget \
"
```

## U-Boot config (mts-rg500_defconfig)

```
CONFIG_ARM=y
CONFIG_TARGET_MTS_RG500=y
CONFIG_ARCH_MEDIATEK=y
CONFIG_SYS_TEXT_BASE=0x41E00000
CONFIG_DEFAULT_DEVICE_TREE="mts-rg500"
CONFIG_CMD_BOOTZ=y
CONFIG_CMD_DHCP=y
CONFIG_CMD_MTD=y
CONFIG_CMD_USB=y
CONFIG_DM_ETH=y
CONFIG_ETH_DESIGNWARE=y
CONFIG_MTD=y
CONFIG_USB=y
CONFIG_USB_XHCI_HCD=y
CONFIG_DM_GPIO=y
CONFIG_I2C=y
CONFIG_PCIE_DW=y
CONFIG_PCI=y
```

## CWMP TR-069 agent (cwmp-agent_1.0.bb)

```bitbake
DESCRIPTION = "MTS CWMP/TR-069 Agent"
LICENSE = "GPL-2.0"
SRC_URI = "file://cwmp-agent.c file://cwmp.service"
do_install() {
    install -d ${D}${sbindir}
    install -m 0755 cwmp-agent ${D}${sbindir}/cwmp-agent
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 cwmp.service ${D}${systemd_unitdir}/system/cwmp.service
}
SYSTEMD_SERVICE:cwmp-agent = "cwmp.service"
```

## WiFi 6 firmware (mt76-firmware_2024.bb)

```bitbake
DESCRIPTION = "MediaTek MT76 WiFi 6 Firmware"
LICENSE = "MIT"
SRC_URI = "https://git.kernel.org/.../mt76/mt7981.eap https://git.kernel.org/.../mt76/mt7981_wa.bin https://git.kernel.org/.../mt76/mt7981_rom_patch.bin"
do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware/mt76
    install -m 0644 mt7981.eap ${D}${nonarch_base_libdir}/firmware/mt76/
    install -m 0644 mt7981_wa.bin ${D}${nonarch_base_libdir}/firmware/mt76/
    install -m 0644 mt7981_rom_patch.bin ${D}${nonarch_base_libdir}/firmware/mt76/
}
```

## Build commands

```bash
# Setup OpenWrt
git clone https://git.openwrt.org/openwrt/openwrt.git openwrt
cd openwrt
./scripts/feeds update -a
./scripts/feeds install -a

# Add meta-mts-openwrt layer
cp -r ${MTS_PROJECT}/linux/meta-mts-openwrt package/mt

# Configure target
make menuconfig
# Target System: MediaTek Ralink MIPS
# Subtarget: MediaTek MT762x
# Target Profile: MTS RG-500

# Build
make -j$(nproc) download
make -j$(nproc)

# Output:
# bin/targets/mediatek/filogic830/mts-rg500-squashfs-sysupgrade.bin
# bin/targets/mediatek/filogic830/mts-rg500-ext4-rootfs.img
```
