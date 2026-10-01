# OpenWrt Layer for MTS-RG-500 Residential Gateway

## Обзор

Этот документ описывает OpenWrt-слой для MTS-RG-500 Residential Gateway на базе MediaTek MT7981.

## Структура слоёв

```
openwrt/
├── target/
│   └── arch/
│       └── mips/
│           └── mt7981/
│               ├── base-files/
│               │   ├── Makefile
│               │   ├── boards/
│               │   │   └── mts-rg500/
│               │   │       ├── boot.sh
│               │   │       ├── filesystem.sh
│               │   │       ├── postinst.sh
│               │   │       └── sysupgrade.sh
│               │   └── files/
│               │       ├── etc/
│               │       │   ├── config/
│               │       │   │   ├── network
│               │       │   │   ├── wireless
│               │       │   │   ├── system
│               │       │   │   ├── led
│               │       │   │   └── mts-gpon
│               │       │   └── network/
│               │       │       ├── interfaces
│               │       │       ├── wireless.cfg
│               │       │       └── hotplug.d/
│               │       │           ├── interface/
│               │       │           │   └── 40-gpon
│               │       │           └── firmware/
│               │       │               └── 10-mt7981
│               │       └── proc/
│               │           └── mts/
│               │               ├── gpon
│               │               ├── wifi
│               │               └── thermal
│               ├── kernel/
│               │   ├── modules/
│               │   │   └── mt7981.mk
│               │   └── configs/
│               │       └── config-5.15
│               └── images/
│                   ├── Makefile
│                   └── mts-rg500.mk
├── package/
│   └── mts/
│       ├── mts-gpon-driver/
│       │   ├── Makefile
│       │   └── src/
│       ├── mts-wifi-mgr/
│       │   ├── Makefile
│       │   └── src/
│       ├── mts-tr069/
│       │   ├── Makefile
│       │   └── src/
│       ├── mts-voip/
│       │   ├── Makefile
│       │   └── src/
│       ├── mts-iptv/
│       │   ├── Makefile
│       │   └── src/
│       └── mts-rg-api/
│           ├── Makefile
│           └── src/
└── files/
    └── etc/
        └── mts/
            ├── gpon.conf
            ├── wifi.conf
            ├── tr069.conf
            └── rg-api.conf
```

## Конфигурация сети

### `/etc/config/network`

```
config interface 'loopback'
    option proto 'static'
    option ipaddr '127.0.0.1'
    option netmask '255.0.0.0'
    option device 'lo'

config globals 'globals'
    option ula_prefix 'fd12:3456:789a::/48'

config interface 'lan'
    option type 'bridge'
    option proto 'static'
    option ipaddr '192.168.1.1'
    option netmask '255.255.255.0'
    option gateway '192.168.1.1'
    option dns '192.168.1.1'
    option device 'eth0 eth1'
    option ip6assign '60'

config interface 'wan'
    option proto 'pppoe'
    option device 'eth2'
    option username 'mts-pppoe'
    option password '********'
    option peerdns '1'
    option defaultroute '1'
    option ipv6 'auto'

config interface 'gpon'
    option type 'gpon-pon'
    option proto 'static'
    option device 'pon0'
    option ipaddr '10.0.0.1'
    option netmask '255.255.255.0'
    option pon_serial '0011223344556677'
    option omci_mgmt '1'
```

### `/etc/config/wireless`

```
config wifi-device 'radio0'
    option type 'mac80211'
    option hwmode '2g'
    option path 'platform/18040000.pcie/pci0000:00/0000:00:00.0/0000:01:00.0'
    option channel '6'
    option band '2g'
    option htmode 'HT40'
    option country '00'
    option disabled '0'

config wifi-iface 'wifinet0'
    option device 'radio0'
    option network 'lan'
    option mode 'ap'
    option ssid 'MTS-RG500-2G'
    option encryption 'sae'
    option key '********'
    option wpa3_transition '1'
    option maxassoc '32'

config wifi-device 'radio1'
    option type 'mac80211'
    option hwmode '5g'
    option path 'platform/18040000.pcie/pci0000:00/0000:00:00.0/0000:01:00.1'
    option channel '36'
    option band '5g'
    option htmode 'VHT80'
    option country '00'
    option disabled '0'

config wifi-iface 'wifinet1'
    option device 'radio1'
    option network 'lan'
    option mode 'ap'
    option ssid 'MTS-RG500-5G'
    option encryption 'sae'
    option key '********'
    option wpa3_transition '1'
    option maxassoc '32'
```

### `/etc/config/mts-gpon`

```
config gpon 'global'
    option enabled '1'
    option onu_id '0011223344556677'
    option omci_version '1.1'
    option auto_reconnect '1'
    option reconnect_delay '30'
    option management_ip '10.0.0.1'
    option omci_mgmt_server '10.0.0.254'
    option omci_mgmt_port '6634'

config gpon_pon 'pon0'
    option phy 'rtl960x'
    option power_level 'medium'
    option tx_bias '4.0mA'
    option wavelength '1490nm'

config gpon_wavelength 'wl1490'
    option type 'TX'
    option wavelength '1490nm'
    option power '0dBm'

config gpon_wavelength 'wl1550'
    option type 'RF'
    option wavelength '1550nm'
    option power '2dBm'
```

## Kernel modules

### `mt7981.mk`

```makefile
# MediaTek MT7981 kernel modules for OpenWrt

define KernelPackage/mt7981-core
  SUBMENU: Other modules
  TITLE: MediaTek MT7981 SoC core driver
  DEPENDS: +kmod/gpio-core +kmod/i2c-core +kmod/spi-gpio
  FILES: $(LINUX_DIR)/mt7981/mt7981-core.ko
  AUTOLOAD: auto
endef

define KernelPackage/mt7981-net
  SUBMENU: Other modules
  TITLE: MediaTek MT7981 Ethernet driver
  DEPENDS: +kmod-mac80211 +kmod-usb3
  FILES: $(LINUX_DIR)/mt7981/mt7981-net.ko
  AUTOLOAD: auto
endef

define KernelPackage/mt7981-wifi
  SUBMENU: Other modules
  TITLE: MediaTek MT7981 WiFi 6 driver (MT76)
  DEPENDS: +kmod-mac80211 +kmod-cfg80211
  FILES: $(LINUX_DIR)/mt7981/mt7981-wifi.ko
  AUTOLOAD: auto
endef

define KernelPackage/mt-gpon
  SUBMENU: Other modules
  TITLE: MTS GPON driver (RTL960x)
  DEPENDS: +kmod-i2c-core
  FILES: $(LINUX_DIR)/mts/mt-gpon.ko
  AUTOLOAD: auto
endef

define KernelPackage/mt-wdm
  SUBMENU: Other modules
  TITLE: MTS WDM/OMCI driver
  DEPENDS: +kmod-mt-gpon
  FILES: $(LINUX_DIR)/mts/mt-wdm.ko
  AUTOLOAD: auto
endef

define KernelPackage/mt-omci
  SUBMENU: Other modules
  TITLE: MTS OMCI management daemon
  DEPENDS: +kmod-mt-wdm +libubus +libuci
  FILES: $(LINUX_DIR)/mts/mt-omci.ko
  AUTOLOAD: auto
endef

$(eval $(call KernelPackage,mt7981-core))
$(eval $(call KernelPackage,mt7981-net))
$(eval $(call KernelPackage,mt7981-wifi))
$(eval $(call KernelPackage,mt-gpon))
$(eval $(call KernelPackage,mt-wdm))
$(eval $(call KernelPackage,mt-omci))
```

## Базовые образы

### `mts-rg500.mk`

```makefile
# MTS-RG500 image generation

MTS_RG500_NAME := MTS-RG500
MTS_RG500_IMAGES := sysupgrade-squashfs-sysupgrade.bin
MTS_RG500_KERNEL := zImage
MTS_RG500_ROOTFS := squashfs

define Device/mts-rg500
  DEVICE_VENDOR := MTS Routers
  DEVICE_MODEL := RG-500 Residential Gateway
  DEVICE_VARIANT := GPON
  DEVICE_PACKAGES := kmod-mt7981-core kmod-mt7981-net kmod-mt7981-wifi \
                     kmod-mt-gpon kmod-mt-wdm kmod-mt-omci \
                     mts-gpon-driver mts-wifi-mgr mts-tr069 \
                     mts-voip mts-iptv mts-rg-api \
                     luci-app-mts-rg \
                     uboot-envtools \
                     fstools \
     ip-full iproute2 ipk iptables dnsmasq hostapd-common wpa-cli \
     ppp ppp-mod-pppoe mac80211-regdb \
     curl wget-ssl jshn jsonfilter \
     usbcore usbmodem kmod-usb-ohci kmod-usb-uhci \
     kmod-usb2 kmod-usb3 kmod-usb-net kmod-usb-net-cdc-ether \
     kmod-usb-net-rtl8150 kmod-usb-net-rtl8152 \
     kmod-i2c-core kmod-i2c-gpio \
     kmod-spi-gpio kmod-spi-bitbang \
     kmod-thermal kmod-hwmon-core \
     kmod-fs-ext4 kmod-fs-fuse \
     e2fsprogs resize2fs \
     coreutils coreutils-nohup coreutils-base64 \
     bash ash-completion attr bzip2 \
     netifd dnsmasq-full
  IMAGES := $(MTS_RG500_IMAGES)
  KERNEL := $(MTS_RG500_KERNEL)
  KERNEL_INITRD :=
  ROOTFS := $(MTS_RG500_ROOTFS)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBOOT_ENVFLAGS := mips
  DEVICE_DTS := mt7981-an600 mt7981-rg500
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
endef

TARGET_DEVICES += mts-rg500
```

## Скрипты загрузки

### `boot.sh`

```bash
#!/bin/sh
# Boot script for MTS-RG500

# Initialize hardware
mt7981_hw_init() {
    # Reset GPON PHY
    i2cset -f -y 0 0x40 0x01 0x01
    sleep 1
    i2cset -f -y 0 0x40 0x01 0x00
    
    # Initialize WiFi RF
    echo 1 > /sys/class/leds/mts-rg500:green:wlan2g/brightness
    echo 1 > /sys/class/leds/mts-rg500:green:wlan5g/brightness
    
    # Enable GPON LED
    echo 1 > /sys/class/leds/mts-rg500:green:gpon/brightness
}

# Setup network interfaces
mt7981_setup_network() {
    # Bring up GMAC0 (LAN)
    ip link set eth0 up
    ip link set eth1 up
    
    # Create bridge
    brctl addbr br-lan
    brctl addif br-lan eth0
    brctl addif br-lan eth1
    ip link set br-lan up
    
    # Configure WAN (GMAC1)
    ip link set eth2 up
    
    # Initialize GPON
    if [ -f /sys/class/gpon/pon0/status ]; then
        echo 1 > /sys/class/gpon/pon0/enable
    fi
}

# Main
mt7981_hw_init
mt7981_setup_network
```

## Package: mts-gpon-driver

```makefile
# package/mts/mts-gpon-driver/Makefile

include $(TOPDIR)/rules.mk

PKG_NAME:=mts-gpon-driver
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_MAINTAINER:=MTS Router Team
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/mts-gpon-driver
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=MTS GPON driver for RTL960x
  DEPENDS:=+kmod-i2c-core
endef

define Package/mts-gpon-driver/description
  Kernel driver for Realtek RTL960x GPON PHY on MTS-RG500
endef

EXTRA_KCONFIG:= \
    CONFIG_MTS_GPON=y \
    CONFIG_MTS_WDM=y \
    CONFIG_MTS_OMCI=y

EXTRA_CFLAGS:= \
    -I$(PKG_BUILD_DIR)/include \
    -DMTS_DRIVER_VERSION=\"$(PKG_VERSION)\"

define Build/Prepare
    mkdir -p $(PKG_BUILD_DIR)
    cp -r $(CURDIR)/src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
    $(MAKE) -C "$(LINUX_DIR)" \
        SUBDIRS="$(PKG_BUILD_DIR)" \
        CONFIG_MTS_GPON=$(CONFIG_MTS_GPON) \
        CONFIG_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
        modules
endef

define Package/mts-gpon-driver/install
    $(INSTALL_DIR) $(1)/lib/modules/$(KERNEL_VERSION)
    $(INSTALL_DATA) $(PKG_BUILD_DIR)/mt-gpon.ko $(1)/lib/modules/$(KERNEL_VERSION)/
    $(INSTALL_DATA) $(PKG_BUILD_DIR)/mt-wdm.ko $(1)/lib/modules/$(KERNEL_VERSION)/
    $(INSTALL_DATA) $(PKG_BUILD_DIR)/mt-omci.ko $(1)/lib/modules/$(KERNEL_VERSION)/
endef

$(eval $(call KernelPackage,mts-gpon-driver))
```

## Скрипт hotplug для GPON

### `/etc/hotplug.d/interface/40-gpon`

```bash
#!/bin/sh

case "$ACTION" in
    ifup)
        if [ "$DEVICE" = "pon0" ]; then
            logger -t mts-gpon "GPON interface $DEVICE up"
            # Initialize OMCI management
            /usr/sbin/omcid -c /etc/mts/gpon.conf &
            # Signal GPON LED
            echo 1 > /sys/class/leds/mts-rg500:green:gpon/brightness
        fi
        ;;
    ifdown)
        if [ "$DEVICE" = "pon0" ]; then
            logger -t mts-gpon "GPON interface $DEVICE down"
            # Stop OMCI management
            killall omcid 2>/dev/null
            # Signal GPON LED
            echo 0 > /sys/class/leds/mts-rg500:green:gpon/brightness
        fi
        ;;
esac
```

## Конфигурация системы

### `/etc/config/system`

```
config 'system'
    option 'hostname' 'mts-rg500'
    option 'timezone' 'UTC'
    option 'tzoffset' '0'
    option 'consolelog' '4'
    option 'log_size' '32768'
    option 'log_ip' ''
    option 'log_port' '514'
    option 'log_proto' 'udp'
    option 'klog' '4'
    option 'utc_time' '1'

config 'led' 'wan'
    option 'name' 'WAN'
    option 'sysfs' 'mts-rg500:green:wan'
    option 'trigger' 'netdev'
    option 'dev' 'eth2'
    option 'mode' 'link'

config 'led' 'wlan2g'
    option 'name' 'WLAN 2.4GHz'
    option 'sysfs' 'mts-rg500:green:wlan2g'
    option 'trigger' 'netdev'
    option 'dev' 'phy0-ap0'
    option 'mode' 'link'

config 'led' 'wlan5g'
    option 'name' 'WLAN 5GHz'
    option 'sysfs' 'mts-rg500:green:wlan5g'
    option 'trigger' 'netdev'
    option 'dev' 'phy1-ap0'
    option 'mode' 'link'

config 'led' 'gpon'
    option 'name' 'GPON'
    option 'sysfs' 'mts-rg500:green:gpon'
    option 'trigger' 'none'
    option 'default' '1'
```

## Примечания

- Device tree: `residential-gateway/linux/device-tree.dts`
- Kernel config: `linux/meta-mts/recipes-kernel/linux/configs/mts-rg500.cfg`
- Machine config: `linux/meta-mts/conf/machine/mts-rg500.conf`
- OpenWrt layer: `linux/meta-mts-openwrt/`
- GPON PHY: Realtek RTL960x (I2C + OMCI)
- WiFi: MediaTek MT76 (dual-band WiFi 6)
- Ethernet: MT7981 GMAC0/GMAC1
