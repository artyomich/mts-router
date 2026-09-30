# CWMP (TR-069) client for MTS Router devices
# Provides ACS client functionality for remote management

SUMMARY = "MTS CWMP client for TR-069 remote management"
DESCRIPTION = "CWMP (TR-069) ACS client for MTS Router devices. \
    Supports remote configuration, firmware upgrade, diagnostics, \
    and performance monitoring."

LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRC_URI = "git://github.com/mts-router/cwmp-mts.git;branch=main;rev=${SRCREV}"
SRCREV = "abc123def456"

S = "${WORKDIR}/git"

inherit cmake pkgconfig

DEPENDS = "libcurl libxml2 libubox libuci libfastjson openssl"

do_configure:append() {
    # Configure CWMP settings
    mkdir -p ${B}/config
    cat > ${B}/config/cwmp.conf <<EOF
[general]
device_id = ${MACHINE}
manufacturer = MTS
model = ${MACHINE}
firmware = ${PV}

[acs]
url = ${MTS_CWMP_ACS_URL:-https://acs.mts-router.local}
port = ${MTS_CWMP_PORT:-7547}
https_port = ${MTS_CWMP_HTTPS_PORT:-443}
username = ${MTS_CWMP_USERNAME:-admin}
password = ${MTS_CWMP_PASSWORD:-}
periodic_interval = ${MTS_CWMP_INTERVAL:-3600}
periodic_enabled = ${MTS_CWMP_PERIODIC_ENABLED:-false}

[connection]
max_connections = ${MTS_CWMP_MAX_CONNECTIONS:-10}
timeout = 30
retry_count = 3
retry_delay = 5

[security]
tls_version = "1.2"
ca_file = /etc/ssl/certs/ca-certificates.crt
cert_file = /etc/cwmp/client.crt
key_file = /etc/cwmp/client.key

[diagnostics]
ping_enabled = true
traceroute_enabled = true
port_scan_enabled = true
dns_lookup_enabled = true

[firmware]
auto_upgrade = true
upgrade_url = ${MTS_CWMP_UPGRADE_URL:-https://firmware.mts-router.local}
checksum_type = sha256

[logging]
level = info
file = /var/log/cwmp.log
max_size = 10M
rotate_count = 5
EOF
}

do_install() {
    install -d ${D}${sysconfdir}/cwmp
    install -d ${D}${sysconfdir}/default
    install -d ${D}${sysconfdir}/init.d
    install -d ${D}${bindir}
    install -d ${D}${sbindir}
    install -d ${D}${libdir}/cwmp
    install -d ${D}${systemd_system_unitdir}

    install -m 0644 ${B}/config/cwmp.conf ${D}${sysconfdir}/cwmp/cwmp.conf
    install -m 0755 ${B}/src/cwmpd ${D}${sbindir}/cwmpd
    install -m 0755 ${B}/src/cwmp-cli ${D}${bindir}/cwmp-cli
    install -m 0644 ${FILESEXTRAPATHS}/cwmp.init ${D}${sysconfdir}/init.d/cwmp
    install -m 0644 ${FILESEXTRAPATHS}/cwmp.conf ${D}${sysconfdir}/default/cwmp
    install -m 0644 ${FILESEXTRAPATHS}/cwmp.service ${D}${systemd_system_unitdir}/cwmp.service
}

SYSTEMD_SERVICE:${PN} = "cwmp.service"

FILES:${PN} = "${sbindir}/cwmpd ${bindir}/cwmp-cli ${sysconfdir}/cwmp ${sysconfdir}/init.d/cwmp ${sysconfdir}/default/cwmp ${systemd_system_unitdir}/cwmp.service"

RDEPENDS:${PN} += "libcurl libxml2 libubox libuci libfastjson openssl utils"

pkg_postinst:${PN}() {
    # Create CWMP configuration directory
    if [ -z "$D" ]; then
        mkdir -p /etc/cwmp
        mkdir -p /var/log
        # Enable CWMP service
        systemctl enable cwmp.service 2>/dev/null || true
        systemctl start cwmp.service 2>/dev/null || true
    fi
}

pkg_prerm:${PN}() {
    if [ -z "$D" ]; then
        systemctl stop cwmp.service 2>/dev/null || true
        systemctl disable cwmp.service 2>/dev/null || true
    fi
}

INSANE_SKIP:${PN} += "already-stripped"
