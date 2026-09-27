# MTS-RG-500 Residential Gateway OpenWrt Image
# MediaTek MT7981 ARM64 + RTL960x GPON
# OpenWrt-based with WiFi 6, VoIP, IPTV

SUMMARY = "MTS-RG-500 Residential Gateway OpenWrt image"
DESCRIPTION = "Complete OpenWrt image for MTS-RG-500 Residential Gateway with WiFi 6 (MT76), GPON (RTL960x), VoIP (Asterisk), IPTV, and TR-069 support"

require mts-image-common.inc

LICENSE = "MIT"

# ============================================================
# OpenWrt configuration
# ============================================================
OPENWRT_SUPPORT = "1"
OPENWRT_VERSION = "23.05"
OPENWRT_TARGET = "mediatek/mt7981"
OPENWRT_SUBTARGET = "mt7981"

# ============================================================
# Image files
# ============================================================
IMAGE_FSTYPES = "ext4.gz wic.gz"
IMAGE_ROOTFS_SIZE = "2097152"
IMAGE_ROOTFS_MAXSIZE = "4194304"

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
# MTS-specific packages for Residential Gateway
# ============================================================
IMAGE_INSTALL:append = " \
    mts-api-server \
    mts-rg500-firmware \
    mts-mt7981-driver \
    mts-rtl960x-driver \
    mts-mt76-firmware \
    mts-mt76-kmod \
    mts-mt76-wifi \
    mts-mt76-phy \
    mts-mt76-eta \
    mts-mt76-efusa \
    mts-mt76-connac \
    mts-mt76-connac3 \
    cwmpd \
    asterisk \
    asterisk-core-apps \
    asterisk-core-modules \
    asterisk-codecs \
    asterisk-channels \
    asterisk-pjsip \
    asterisk-owasp \
    asterisk-res-acl \
    asterisk-res-adaptative \
    asterisk-res-amd \
    asterisk-res-audiosocket \
    asterisk-res-b2bua \
    asterisk-res-callerid \
    asterisk-res-clioriginate \
    asterisk-res-compressfile \
    asterisk-res-convert \
    asterisk-res-dhcp \
    asterisk-res-digiumphone \
    asterisk-res-disa \
    asterisk-res-dns \
    asterisk-res-dnsmgr \
    asterisk-res-endpoint-mapper \
    asterisk-res-event-multicast \
    asterisk-res-fax \
    asterisk-res-fifo \
    asterisk-res-geolocation \
    asterisk-res-http-acl \
    asterisk-res-http-cache \
    asterisk-res-ice \
    asterisk-res-lockdown \
    asterisk-res-moh-ropes \
    asterisk-res-moh-sounds \
    asterisk-res-musiconhold \
    asterisk-res-native-mux \
    asterisk-res-protobuf \
    asterisk-res-pva-freeswitch \
    asterisk-res-realtime \
    asterisk-res-rtp-engine \
    asterisk-res-sdi \
    asterisk-res-security \
    asterisk-res-skinny \
    asterisk-res-sms \
    asterisk-res-soc \
    asterisk-res-sorcery \
    asterisk-res-speech \
    asterisk-res-statsf \
    asterisk-res-stasis \
    asterisk-res-timing-bfd \
    asterisk-res-timing-pthread \
    asterisk-res-timing-timerfd \
    asterisk-res-udptl \
    asterisk-res-voiceurl \
    asterisk-res-xmpp \
    asterisk-bin-asterisk \
    asterisk-bin-asteriskctl \
    asterisk-bin-cdr \
    asterisk-bin-confbridge \
    asterisk-bin-dahdiconf \
    asterisk-bin-dahdidump \
    asterisk-bin-dahdiaudiometer \
    asterisk-bin-dahdisharedmemtest \
    asterisk-bin-kztdiadumpinfo \
    asterisk-bin-mmlsort \
    asterisk-bin-smsq \
    asterisk-bin-spandspimgtest \
    asterisk-bin-ztdummyconf \
    asterisk-bin-ztmonitor \
    asterisk-bin-zttstcc \
    asterisk-bin-zttstccrt \
    asterisk-bin-zttstccrtf \
    asterisk-bin-zttstccrta \
    asterisk-bin-zttstccrtg \
    asterisk-bin-zttstccrtgf \
    asterisk-bin-ztregexconf \
    asterisk-bin-zttranslate \
    asterisk-bin-ztloopback \
    asterisk-bin-zttonezone \
    asterisk-bin-ztfilter \
    asterisk-bin-ztscanadi \
    asterisk-bin-ztcfg \
    asterisk-bin-ztcontrol \
    asterisk-bin-ztdigest \
    asterisk-bin-zttest \
    asterisk-bin-zttonegen \
    asterisk-bin-ztconf \
    asterisk-bin-ztinfo \
    asterisk-bin-ztmonitor \
    asterisk-bin-ztloopback \
    asterisk-bin-ztscanadi \
    asterisk-bin-ztcfg \
    asterisk-bin-ztcontrol \
    asterisk-bin-ztdigest \
    asterisk-bin-zttest \
    asterisk-bin-zttonegen \
    asterisk-bin-ztconf \
    asterisk-bin-ztinfo \
    luci \
    luci-base \
    luci-compat \
    luci-ssl \
    luci-proto-ipv6 \
    luci-proto-ppp \
    luci-proto-relay \
    luci-app-cwmp \
    luci-app-voip \
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
    kmod-rtw89-8852ae \
    kmod-mt7915e \
    kmod-mt7981-wifi \
    kmod-mt7981-eth \
    kmod-mt7530 \
    kmod-rtl8366-smi \
    kmod-rtl8366rb \
    kmod-rtl8367b \
    kmod-rtl8367b-smi \
    kmod-rtl8366rb-smi \
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
IMAGE_POSTPROCESS_COMMAND += " mts-rg500-boot-config;"

# ============================================================
# Boot configuration for RG-500
# ============================================================
mts-rg500-boot-config() {
    install -d ${DEPLOY_DIR_IMAGE}/extlinux
    cat > ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf << EOF
LABEL mts-rg500
    MENU LABEL MTS-RG-500 Residential Gateway
    KERNEL /boot/vmlinuz
    APPEND root=/dev/mmcblk0p2 rw rootwait console=ttyS0,115200n8 mts.board=rg500 mts.os=openwrt mt7981.wifi=on rtl960x.gpon=on
    INITRD /boot/initrd.img
EOF
    cp ${DEPLOY_DIR_IMAGE}/extlinux/extlinux.conf ${DEPLOY_DIR_IMAGE}/
}

# ============================================================
# Image deployment
# ============================================================
do_deploy:append() {
    local img_name="mts-rg500-image-${PV}.ext4.gz"

    cp ${DEPLOY_DIR_IMAGE}/images/mts-rg500/${IMAGE_ROOTFS} ${DEPLOY_DIR_IMAGE}/${img_name}

    cd ${DEPLOY_DIR_IMAGE}
    sha256sum ${img_name} > ${img_name}.sha256

    find ${D} -type f | LC_ALL=C sort > ${DEPLOY_DIR_IMAGE}/mts-rg500-manifest.txt
}

COMPATIBLE_MACHINE = "^mts-rg500$"
IMAGE_NAME = "mts-rg500-image"
PV = "1.0.0"
