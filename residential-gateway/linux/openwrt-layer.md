# OpenWrt Layer для MTS-RG-500 (Residential Gateway)

## 1. Структура meta-mts-openwrt слоя

```
meta-mts-openwrt/
├── conf/
│   ├── layer.conf                    # Определение слоя
│   └── mts-targets.conf              # Целевые платформы
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
├── recipes-network/
│   ├── dnsmasq/
│   │   └── dnsmasq_%.bbappend        # DNS/DHCP
│   ├── strongswan/
│   │   └── strongswan_%.bbappend     # IPsec
│   └── frrouting/
│       └── frr_%.bbappend            # FRRouting
├── recipes-extended/
│   ├── cwmp/
│   │   └── cwmp-agent_1.0.bb        # TR-069 агент
│   ├── asterisk/
│   │   └── asterisk_20.bb           # VoIP (Asterisk)
│   └── miniupnpd/
│       └── miniupnpd_2.2.bb         # UPnP
├── recipes-graphics/
│   └── luuci/
│       └── luci_24.bb               # Web UI
├── recipes-ml/
│   └── mt76-firmware/
│       └── mt76-firmware_2024.bb    # WiFi 6 firmware
└── classes/
    └── mts-gateway.bbclass           # Базовый класс
```

## 2. Конфигурация слоя

### conf/layer.conf

```bitbake
# Meta-mts-openwrt layer
LAYERSERIES_COMPAT_meta-mts-openwrt = "kirkstone morty"

BBPATH .= ":${LAYERDIR}"

BBFILES += "${LAYERDIR}/recipes-*/*/*.bb ${LAYERDIR}/recipes-*/*/*.bbappend"

LAYERDEPENDS_meta-mts-openwrt = "core openwrt"

LAYERSERIES_COMPAT_openwrt = "kirkstone morty"
```

### conf/mts-targets.conf

```bitbake
# Target platforms for MTS gateways
MTS_TARGETS = "mts-rg500"

# Image types
MTS_IMAGE_TYPES = "ext4 squashfs"

# Kernel version
MTS_KERNEL_VERSION = "6.6"

# OpenWrt target
MTS_OPENWRT_TARGET = "mediatek"
MTS_OPENWRT_SUBTARGET = "filogic830"
```

## 3. Machine configuration

### conf/machine/mts-rg500.conf

```bitbake
# MTS Residential Gateway RG-500 Machine Configuration
include conf/machine/include/mts-base.inc

# Machine name
MACHINE = "mts-rg500"

# SoC
SOC_FAMILY = "mtk:mt7981"

# Kernel
KERNEL_DEVICETREE = "mediatek/mts-rg500.dtb"

# Bootloader
UBOOT_MACHINE = "mts-rg500_defconfig"

# Filesystem
IMAGE_FSTYPES = "ext4.gz tar.gz"

# Root filesystem size
IMAGE_ROOTFS_SIZE = "65536"

# Serial console
SERIAL_CONSOLE = "115200 /dev/ttyMT0"

# Network interfaces
MTS_ETH_INTERFACES = "eth0 eth1 eth2 eth3 eth4"
MTS_WIFI_INTERFACES = "mt7602e mt7615e"
MTS_GPON_INTERFACE = "pon0"

# Features
MACHINE_FEATURES += "wifi gpon voip iptv usb"

# Packages
IMAGE_INSTALL:append = " \
    dnsmasq \
    strongswan \
    asterisk \
    miniupnpd \
    cwmp-agent \
    mt76-firmware \
    luci \
"
```

## 4. Kernel configuration

### recipes-kernel/linux/configs/mts-rg500.cfg

```bitbake
# MTS-RG500 Kernel Configuration

# MediaTek MT7981
CONFIG_ARCH_MEDIATEK=y
CONFIG_MTK_SOC_MT7981=y
CONFIG_MTK_PPE=y
CONFIG_MTK_EPHY=y
CONFIG_MTK_ETH=y
CONFIG_MTK_WMAC=y
CONFIG_MTK_FRAC_TX=y

# GPON
CONFIG_GPON_RTL960X=y
CONFIG_OMCI=y
CONFIG_GPON_WDM=y

# VoIP
CONFIG_SND_SOC=y
CONFIG_SND_SOC_MT7981=y
CONFIG_SND_SOC_AIC3254=y
CONFIG_PDM_DAC=y

# USB
CONFIG_USB=y
CONFIG_USB_EHCI_HCD=y
CONFIG_USB_OHCI_HCD=y
CONFIG_USB_ACM=y

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
CONFIG_IP_MROUTE=y
CONFIG_IP_MROUTE_MULTICAST=y

# IPsec
CONFIG_CRYPTO_AEAD=y
CONFIG_CRYPTO_AES=y
CONFIG_CRYPTO_SHA256=y
CONFIG_XFRM=y
CONFIG_XFRM_USER=y

# TR-069/CWMP
CONFIG_CWMP=y

# Filesystem
CONFIG_EXT4_FS=y
CONFIG_SQUASHFS=y
CONFIG_MTD=y
CONFIG_MTD_NAND=y

# Debug
CONFIG_DEBUG_INFO=y
CONFIG_KPROBES=y
```

## 5. Device Tree

### recipes-kernel/linux/files/device-tree/mts-rg500.dts

```dts
/dts-v1/;
#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/input/input.h>

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

    chosen {
        stdout-path = "serial0:115200n8";
    };

    memory {
        reg = <0x0 0x40000000 0 0x10000000>;
    };

    leds {
        compatible = "gpio-leds";
        sys_led: sys {
            label = "mts-rg500:amber:sys";
            gpios = <&gpio 12 GPIO_ACTIVE_LOW>;
        };
        pon_led: pon {
            label = "mts-rg500:green:pon";
            gpios = <&gpio 15 GPIO_ACTIVE_LOW>;
        };
    };

    buttons {
        compatible = "gpio-keys";
        reset_btn: reset {
            label = "Reset";
            gpios = <&gpio 20 GPIO_ACTIVE_LOW>;
            linux,code = <KEY_RESTART>;
            debounce-interval = <60>;
        };
    };
};

&uart0 {
    status = "okay";
};

&eth {
    gmac0: mac@0 {
        compatible = "mediatek,eth-mac";
        reg = <0>;
        phy-mode = "rgmii";

        fixed-link {
            speed = <1000>;
            full-duplex;
            pause;
        };
    };

    gmac1: mac@1 {
        compatible = "mediatek,eth-mac";
        reg = <1>;
        phy-mode = "rgmii";

        fixed-link {
            speed = <1000>;
            full-duplex;
            pause;
        };
    };
};

&ethsw {
    status = "okay";
    mediatek,ethsw;
};

&wmac {
    status = "okay";
    mediatek,mtd-eeprom = <&factory 0x0>;
};

&pci {
    status = "okay";
};

&wifi {
    status = "okay";
    mediatek,mtd-eeprom = <&factory 0x8000>;
};

&gp {
    gp0 {
        status = "okay";
        rtk,pon-ont;
    };
};
```

## 6. Image recipe

### recipes-core/images/mts-residential-image.bb

```bitbake
require mts-image-common.inc

IMAGE_NAME = "mts-rg500-image"

IMAGE_INSTALL:append = " \
    kernel-image \
    uboot-mts \
    dtb-mts-rg500 \
    \
    busybox \
    dropbear \
    iproute2 \
    iputils \
    ethtool \
    \
    dnsmasq \
    dhcp-server \
    \
    strongswan \
    ipsec-tools \
    \
    asterisk \
    libpri \
    dahdi-tools \
    \
    miniupnpd \
    cwmp-agent \
    \
    mt76-firmware \
    \
    luci-base \
    luci-app-firewall \
    luci-app-dhcp \
    luci-app-ddns \
    luci-app-samba \
    \
    kmod-usb-core \
    kmod-usb-ohci \
    kmod-usb-uhci \
    kmod-usb2 \
    kmod-usb3 \
    \
    kmod-gpio-button-hotplug \
    kmod-leds-gpio \
    kmod-sound-mt7981 \
    \
    fstools \
    block-mount \
    e2fsprogs \
    \
    netifd \
    odhcpd \
    odhcp6c \
    \
    ca-certificates \
    openssl-util \
    curl \
    wget \
"

IMAGE_CLASSES:append = " mts-gateway"

do_image[depends] += "u-boot-mts:do_deploy"

FILES:${IMAGE_NAME} = " \
    /boot/* \
    /etc/* \
    /lib/* \
    /usr/* \
    /sbin/* \
    /bin/* \
"
```

## 7. U-Boot configuration

### recipes-bsp/u-boot/files/mts-rg500_defconfig

```
CONFIG_ARM=y
CONFIG_TARGET_MTS_RG500=y
CONFIG_ARCH_MEDIATEK=y
CONFIG_SYS_TEXT_BASE=0x41E00000
CONFIG_NR_DRAM_BANKS=1
CONFIG_ENV_SIZE=0x2000
CONFIG_DEFAULT_DEVICE_TREE="mts-rg500"
CONFIG_CONSOLE_MUX=y
CONFIG_SYS_LOAD_ADDR=0x40000000
CONFIG_ENV_OFFSET=0x3E0000
CONFIG_ENV_OFFSET_REDUND=0x3C0000
CONFIG_CMD_BOOTZ=y
CONFIG_CMD_DHCP=y
CONFIG_CMD_MTD=y
CONFIG_CMD_NAND=y
CONFIG_CMD_USB=y
CONFIG_CMD_GPIO=y
CONFIG_CMD_I2C=y
CONFIG_CMD_PCI=y
CONFIG_DM_ETH=y
CONFIG_ETH_DESIGNWARE=y
CONFIG_MTD=y
CONFIG_DM_MTD=y
CONFIG_MTD_NAND_MT27xx=y
CONFIG_USB=y
CONFIG_USB_XHCI_HCD=y
CONFIG_USB_EHCI_HCD=y
CONFIG_USB_OHCI_HCD=y
CONFIG_USB_STORAGE=y
CONFIG_DM_GPIO=y
CONFIG_I2C=y
CONFIG_DM_I2C=y
CONFIG_PCIE_DW=y
CONFIG_PCI=y
```

## 8. CWMP (TR-069) agent

### recipes-extended/cwmp/cwmp-agent_1.0.bb

```bitbake
DESCRIPTION = "MTS CWMP/TR-069 Agent for Residential Gateways"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=1234567890abcdef"

SRC_URI = "file://cwmp-agent.c \
           file://cwmp-config.h \
           file://acs-client.c \
           file://omci-bridge.c \
           file://Makefile \
           file://cwmp.service \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 cwmp-agent ${D}${sbindir}/cwmp-agent
    
    install -d ${D}${sysconfdir}/cwmp
    install -m 0644 cwmp-config.h ${D}${sysconfdir}/cwmp/config.h
    
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 cwmp.service ${D}${systemd_unitdir}/system/cwmp.service
}

SYSTEMD_SERVICE:cwmp-agent = "cwmp.service"

FILES:${PN} = " \
    ${sbindir}/cwmp-agent \
    ${sysconfdir}/cwmp/* \
    ${systemd_unitdir}/system/cwmp.service \
"
```

## 9. Asterisk (VoIP) configuration

### recipes-extended/asterisk/asterisk_20.bb

```bitbake
DESCRIPTION = "MTS VoIP Server (Asterisk) for RG-500"
LICENSE = "GPL-2.0"

SRC_URI = "gitsourceforge://asterisk/asterisk asterisk-%{version}.tar.gz"

S = "${WORKDIR}/asterisk-${PV}"

EXTRA_OEMAKE = "PREFIX=${prefix} \
    BINDIR=${sbindir} \
    CONFDIR=${sysconfdir}/asterisk \
    LIBDIR=${libdir}/asterisk/modules \
    RUNTIMEDIR=/var/run/asterisk \
    DATADIR=${datadir}/asterisk \
    CURDIR=${WORKDIR} \
    CFLAGS+=\"-O2 -march=armv8-a -mtune=cortex-a53\" \
"

do_configure:append() {
    oe_runmake menuselect.makeconf
}

do_install:append() {
    install -d ${D}${sysconfdir}/asterisk
    cp -r ${S}/configs/asterisk.conf \
           ${S}/configs/sip.conf \
           ${S}/configs/extensions.conf \
           ${D}${sysconfdir}/asterisk/
}
```

## 10. WiFi 6 firmware

### recipes-ml/mt76-firmware/mt76-firmware_2024.bb

```bitbake
DESCRIPTION = "MediaTek MT76 WiFi 6 Firmware"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=abc123"

SRC_URI = "https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/plain/mt76/mt7981.eap \
           https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware/plain/mt76/mt7981_wa.bin \
           https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware/plain/mt76/mt7981_rom_patch.bin \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware/mt76
    install -m 0644 mt7981.eap ${D}${nonarch_base_libdir}/firmware/mt76/
    install -m 0644 mt7981_wa.bin ${D}${nonarch_base_libdir}/firmware/mt76/
    install -m 0644 mt7981_rom_patch.bin ${D}${nonarch_base_libdir}/firmware/mt76/
}

FILES:${PN} = "${nonarch_base_libdir}/firmware/mt76/*"
```

## 11. Build commands

```bash
# Setup OpenWrt
git clone https://git.openwrt.org/openwrt/openwrt.git openwrt
cd openwrt

# Download feeds
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

# Output
# bin/targets/mediatek/filogic830/mts-rg500-squashfs-sysupgrade.bin
# bin/targets/mediatek/filogic830/mts-rg500-ext4-rootfs.img
```

## 12. OpenWrt package integration

### package/mt/mts-gateway/Makefile

```bitbake
include $(TOPDIR)/rules.mk

PKG_NAME:=mts-gateway
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mts-router/mts-gateway.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_MAINTAINER:=MTS Router Team
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/mts-gateway
    SECTION:=net
    CATEGORY:=Network
    TITLE:=MTS Residential Gateway Management
    DEPENDS:=+libubox +libuci +libubus +libnl-tiny
endef

define Package/mts-gateway/description
    MTS Residential Gateway management daemon for RG-500
endef

define Build/Compile
    $(MAKE) -C $(PKG_BUILD_DIR) \
        CROSS="$(TARGET_CROSS)" \
        CC="$(TARGET_CC)" \
        CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
        LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/mts-gateway/install
    $(INSTALL_DIR) $(1)/usr/sbin
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/mts-gateway $(1)/usr/sbin/
    $(INSTALL_DIR) $(1)/etc/config
    $(INSTALL_DATA) ./files/mts-gateway.conf $(1)/etc/config/mts-gateway
    $(INSTALL_DIR) $(1)/etc/init.d
    $(INSTALL_BIN) ./files/mts-gateway.init $(1)/etc/init.d/mts-gateway
endef

$(eval $(call BuildPackage,mts-gateway))
```
