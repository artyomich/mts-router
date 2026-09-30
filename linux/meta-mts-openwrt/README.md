# meta-mts-openwrt — OpenWrt Layer for MTS Router Devices

OpenWrt-based Yocto layer for MTS residential and enterprise gateways.

## Overview

This layer provides OpenWrt-based image recipes, device trees, and kernel
configurations for the following MTS devices:

- **MTS-RG-500** — Residential Gateway (MT7981 + RTL960x)
- **MTS-ER-1000** — Enterprise Router (S32G3 + TomTom)
- **MTS-OLT-2000** — OLT GPON (Tofino 2 + RTL960x)

## Layer Structure

```
meta-mts-openwrt/
├── conf/
│   └── layer.conf              # Layer configuration
├── classes/
│   ├── mts-openwrt.bbclass     # OpenWrt image class
│   └── mts-firmware.bbclass    # Firmware update class
├── recipes-bsp/
│   ├── u-boot/
│   │   └── u-boot-mts_2024.04.bb
│   └── device-tree/
│       └── mts-device-tree.bb
├── recipes-core/
│   ├── images/
│   │   ├── mts-residential-image.bb
│   │   ├── mts-enterprise-image.bb
│   │   └── mts-olt-gpon-image.bb
│   └── packages/
│       └── mts-drivers/
├── recipes-extended/
│   ├── cwmp/
│   │   └── cwmp-mts_1.8.bb
│   └── mts-drivers/
│       └── mts-drivers_1.0.bb
└── README.md
```

## Configuration

### Add the layer to bblayers.conf

```bash
meta-mts-openwrt \
```

### Set MACHINE

For each device:

```bash
# Residential Gateway
MACHINE = "mts-rg500"

# Enterprise Router
MACHINE = "mts-er1000"

# OLT GPON
MACHINE = "mts-olt2000"
```

## Building

```bash
# Build residential gateway image
bitbake mts-residential-image

# Build enterprise router image
bitbake mts-enterprise-image

# Build OLT GPON image
bitbake mts-olt-gpon-image
```

## Components

### CWMP (TR-069)

The `cwmp-mts` recipe provides:
- ACS client for remote management
- Firmware download and upgrade
- Configuration backup/restore
- Diagnostics (ping, traceroute, port scan)

### MTS Drivers

The `mts-drivers` recipe includes:
- MT7981 Ethernet/WiFi driver
- RTL960x GPON driver
- S32G3 network driver
- TomTom ASIC driver
- Tofino 2 P4 driver

### Device Trees

Device trees for each board are provided in:
- `recipes-bsp/device-tree/files/mts-rg500.dts`
- `recipes-bsp/device-tree/files/mts-er1000.dts`
- `recipes-bsp/device-tree/files/mts-olt2000.dts`

## OpenWrt Packages

The following OpenWrt packages are included:

| Package | Description |
|---------|-------------|
| lwiot-mts | MTS IoT protocol stack |
| mts-omci | OMCI protocol implementation |
| mts-tr069 | TR-069 client |
| mts-aptd | MTS access protocol daemon |
| asterisk-ets | Asterisk with MTS codecs |
| mts-ppp | MTS PPP extensions |
| mts-l2tp | MTS L2TPv3 |
| mts-vpnb | MTS VPN bundle |
| mts-syslog | MTS syslog client |
| mts-dhcp6c | MTS DHCPv6 client |
| mts-ndppd | MTS NDP proxy daemon |
| mts-olsrd | MTS OLSR daemon |
| mts-babeld | MTS Babel routing daemon |
| mts-batman | MTS B.A.T.M.A.N. |
| mts-openndp | MTS OpenNDP |
| mts-nat6 | MTS NAT6 |
| mts-ubusd | MTS ubus daemon |
| mts-procd | MTS procd init |
| mts-firewall3 | MTS firewall3 |
| mts-kmod-switch | MTS switch kernel module |
| mts-switchdev | MTS switchdev |
| mts-wpad | MTS wireless daemon |
| mts-wifi-scripts | MTS WiFi scripts |
| mts-hostapd | MTS hostapd |
| mts-wpad-mini | MTS wpad-mini |
| mts-atheros-scripts | MTS Atheros scripts |
| mts-mwlwifi | MTS mwlwifi driver |
| mts-mt76-kmod | MTS mt76 kernel module |
| mts-mt76-wifi | MTS mt76 WiFi |
| mts-mt76-phy | MTS mt76 PHY |
| mts-ath10k-firmware | MTS ath10k firmware |
| mts-ath10k-phy | MTS ath10k PHY |
| mts-ath10k-scripts | MTS ath10k scripts |
| mts-ath10k-bdaddr | MTS ath10k bdaddr |
| mts-ath10k-qt2 | MTS ath10k qt2 |
| mts-ath10k-small | MTS ath10k small |
| mts-ath10k-ct | MTS ath10k ct |
| mts-ath10k-ct-small | MTS ath10k ct small |
| mts-ath10k-ct-bdaddr | MTS ath10k ct bdaddr |
| mts-ath10k-htc | MTS ath10k htc |
| mts-ath10k-hw | MTS ath10k hw |
| mts-ath10k-mac | MTS ath10k mac |
| mts-ath10k-mac80211 | MTS ath10k mac80211 |
| mts-ath10k-phy2 | MTS ath10k phy2 |
| mts-ath10k-phy2-scripts | MTS ath10k phy2 scripts |
| mts-ath10k-phy2-bdaddr | MTS ath10k phy2 bdaddr |
| mts-ath10k-phy2-qt2 | MTS ath10k phy2 qt2 |
| mts-ath10k-phy2-small | MTS ath10k phy2 small |
| mts-ath10k-phy2-ct | MTS ath10k phy2 ct |
| mts-ath10k-phy2-ct-small | MTS ath10k phy2 ct small |
| mts-ath10k-phy2-ct-bdaddr | MTS ath10k phy2 ct bdaddr |
| mts-ath10k-phy2-hw | MTS ath10k phy2 hw |
| mts-ath10k-phy2-mac | MTS ath10k phy2 mac |
| mts-ath10k-phy2-mac80211 | MTS ath10k phy2 mac80211 |

## License

GPL-2.0
