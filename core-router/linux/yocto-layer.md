# MTS-CR-9000 — Yocto Layer

## 1. Структура meta-mts слоя

```
meta-mts/
├── conf/
│   ├── layer.conf
│   ├── machine/
│   │   └── mts-cr9000.conf
│   └── distro/
│       └── mts-core.conf
├── recipes-core/
│   ├── init-system-helpers/
│   └── systemd/
├── recipes-kernel/
│   ├── linux/
│   │   └── linux-mts_6.6.bb
│   └── dpdk/
│       └── dpdk-mts_23.11.bb
├── recipes-graphics/
│   └── tofino/
│       └── tofino-fw/
│           └── tofino-fw_2.0.bb
└── recipes-networking/
    ├── frrouting/
    │   └── frrouting_9.0.bb
    └── mts-forwarder/
        └── mts-forwarder_1.0.bb
```

## 2. layer.conf

```bitbake
# Yocto Layer Configuration for MTS Core Router

# Path to this layer
BBPATH .= ":${LAYERDIR}"

# Files in this layer
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
            ${LAYERDIR}/recipes-*/*/*.bbappend"

BBFILE_COLLECTIONS += "meta-mts"
BBFILE_PATTERN_meta-mts = "^${LAYERDIR}/"
BBFILE_PRIORITY_meta-mts = "60"

# Dependencies
LAYERSERIES_COMPAT_meta-mts = "kirkstone morty"
```

## 3. machine/mts-cr9000.conf

```bitbake
# Machine configuration for MTS-CR-9000

MACHINE = "mts-cr9000"

# CPU
TUNE_FEATURES = "m64 core-avx2"
TARGET_FPU = ""

# Arch
TARGET_ARCH = "x86_64"
TARGET_OS = "linux"

# Kernel
KERNEL_CLASSES = "kernel-bzimage"
KERNEL_IMAGE_TYPE = "bzImage"
KERNEL_DEVICETREE = ""

# Rootfs
IMAGE_FSTYPES = "ext4"
IMAGE_BOOT_FILES = "bzImage"

# Package manager
PACKAGE_CLASSES = "package_rpm"
INHERIT += "rm_work"

# Networking
DISTRO_FEATURES_append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
DISTRO_FEATURES_BACKFILL_CONSIDERED = "sysvinit"
VIRTUAL-RUNTIME_initscript = ""
```

## 4. distro/mts-core.conf

```bitbake
# Distro configuration for MTS Core Router

DISTRO = "mts-core"
DISTRO_VERSION = "1.0.0"
DISTRO_CODENAME = "tsar"

# Package management
PACKAGE_CLASSES = "package_rpm"
PACKAGECONFIG_pn-qemu-native = "slirp"
PACKAGECONFIG_pn-nativesdk-qemu = "slirp"

# Security
SECURITY_POLICY = "selinux"
DISTRO_FEATURES_append = " selinux"

# Logging
DISTRO_FEATURES_append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
```

## 5. recipes-kernel/linux/linux-mts_6.6.bb

```bitbake
require recipes-kernel/linux/linux-yocto.inc

SRCREV = "v6.6.0-mts"
LINUX_VERSION = "6.6.0"
LINUX_VERSION_EXTENSION = "-mts-core"

FILESEXTRAPATHS:prepend := "${THISDIR}/linux-mts:"

SRC_URI = "git://github.com/mts/linux-mts.git;branch=mts-core-6.6 \
           file://config \
           file://defconfig \
           file://mts-cr9000.dts"

KCONFIG_MODE = "--alldefconfig"

COMPATIBLE_MACHINE = "mts-cr9000"
```

## 6. recipes-kernel/dpdk/dpdk-mts_23.11.bb

```bitbake
SUMMARY = "DPDK for MTS Core Router"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=XXXX"

SRC_URI = "git://github.com/DPDK/dpdk.git;branch=v23.11"

S = "${WORKDIR}/git"

DEPENDS = "meson ninja pkgconfig-native"

PACKAGECONFIG ??= "intel \
                   mlx5 \
                   ixgbef \
                   iavf \
                   mlx4 \
                   bnxt \
                   octeontx2 \
                   sfc \
                   qede \
                   vmxnet3 \
                   vhost \
                   vfio \
                   rxtx_vec_sse \
                   rxtx_vec_avx2 \
                   rxtx_vec_neon \
                   debug \
                   profiling \
                   metrics \
                   trace \
                   telemetry \
                   eal \
                   net \
                   bus \
                   crypto \
                   compress \
                   event \
                   mbuf \
                   ring \
                   hash \
                   bitmap \
                   meter \
                   timer \
                   power \
                   security \
                   rawdev \
                   mempool \
                   metrics \
                   netvsc \
                   net_af_xdp \
                   net_i40e \
                   net_ice \
                   net_ionic \
                   net_lgc \
                   net_mlx5 \
                   net_nfp \
                   net_octeontx2 \
                   net_sfc \
                   net_virtio \
                   net_vmxnet3 \
                   net_xenvirt \
                   net_tap \
                   net_af_packet \
                   net_dpaa \
                   net_dpaa2 \
                   net_failsafe \
                   net_cnxk \
                   net_hns3 \
                   net_lemac \
                   net_mlx4 \
                   net_mlx5 \
                   net_nitrox \
                   net_nxp \
                   net_qede \
                   net_sfc \
                   net_virtio \
                   net_vmxnet3 \
                   net_xen \
                   net_tap \
                   net_af_packet \
                   net_dpaa \
                   net_dpaa2 \
                   net_failsafe \
                   net_cnxk \
                   net_hns3 \
                   net_lemac \
                   net_mlx4 \
                   net_mlx5 \
                   net_nitrox \
                   net_nxp \
                   net_qede \
                   net_sfc \
                   net_virtio \
                   net_vmxnet3 \
                   net_xen"

do_configure() {
    meson setup build \
        -Dprefix=/usr \
        -Dlibdir=lib64 \
        -Dexamples=all \
        -Denable_docs=false \
        -Dwerror=false \
        -Dc_args='-O2 -mavx2' \
        -Ddep_features=intel \
        ${PACKAGECONFIG_CONFARGS}
}

do_compile() {
    ninja -C build
}

do_install() {
    ninja -C build install
}

FILES:${PN} += "/usr/lib/dpdk/*"
FILES:${PN}-dev += "/usr/include/dpdk/*"
```

## 7. recipes-networking/mts-forwarder/mts-forwarder_1.0.bb

```bitbake
SUMMARY = "MTS Core Router Forwarder"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=XXXX"

SRC_URI = "git://github.com/mts/mts-forwarder.git;branch=main"

S = "${WORKDIR}/git"

DEPENDS = "dpdk"

do_configure() {
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DDPDK_DIR=/usr \
        .
}

do_compile() {
    cmake --build build -j${@os.cpu_count()}
}

do_install() {
    cmake --install build
}

FILES:${PN} += "/usr/bin/mts-forwarder"
FILES:${PN}-dev += "/usr/include/mts-forwarder/*"
```

## 8. recipes-networking/frrouting/frrouting_9.0.bb

```bitbake
SUMMARY = "FRRouting for MTS Core Router"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=XXXX"

SRC_URI = "git://github.com/FRRouting/frr.git;branch=frr-9.0"

S = "${WORKDIR}/git"

DEPENDS = "libyang libyang2 json-c libpcre2"

do_configure() {
    autoreconf -fiv
    ./configure \
        --prefix=/usr \
        --sysconfdir=/etc/frr \
        --localstatedir=/var/run/frr \
        --enable-rpki=yes \
        --enable-pmcd=yes \
        --enable-fpm=yes \
        --enable-pfexec=yes \
        --enable-systemd=yes \
        --enable-multi-thread=yes \
        --enable-arpd=no \
        --enable-snmp=no \
        --enable-isisd=yes \
        --enable-ospfclient=yes \
        --enable-ospfapi=yes \
        --enable-ospfapi6=yes \
        --enable-ospfapi6=yes \
        --enable-bfdd=yes \
        --enable-ldpd=yes \
        --enable-static=lib \
        --enable-rtadv=no \
        --enable-distributed-routing=yes \
        --enable-isisd=yes \
        --enable-ospfclient=yes \
        --enable-ospfapi=yes \
        --enable-ospfapi6=yes \
        --enable-bfdd=yes \
        --enable-ldpd=yes \
        --enable-static=lib \
        --enable-rtadv=no \
        --enable-distributed-routing=yes
}

do_compile() {
    make -j${@os.cpu_count()}
}

do_install() {
    make install DESTDIR=${D}
}
```

## 9. recipes-graphics/tofino/tofino-fw/tofino-fw_2.0.bb

```bitbake
SUMMARY = "Intel Tofino 2 Firmware"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=XXXX"

SRC_URI = "git://github.com/IntelP4/tofino-fw.git;branch=v2.0"

S = "${WORKDIR}/git"

do_configure() {
    mkdir -p ${WORKDIR}/tofino-fw
    cp -r firmware/* ${WORKDIR}/tofino-fw/
}

do_install() {
    install -d ${D}/usr/share/tofino-fw
    cp -r ${WORKDIR}/tofino-fw/* ${D}/usr/share/tofino-fw/
}

FILES:${PN} += "/usr/share/tofino-fw/*"
```

## 10. build/conf/local.conf

```bitbake
# Local configuration for MTS Core Router

MACHINE = "mts-cr9000"
DISTRO = "mts-core"

DL_DIR ?= "${TOPDIR}/downloads"
SSTATE_DIR ?= "${TOPDIR}/sstate-cache"
TMPDIR = "${TOPDIR}/tmp"

PARALLEL_MAKE = "-j${@os.cpu_count()}"

BB_NUMBER_THREADS = "${@os.cpu_count()}"

# Security
SECURITY_POLICY = "selinux"

# Logging
DISTRO_FEATURES_append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
DISTRO_FEATURES_BACKFILL_CONSIDERED = "sysvinit"
VIRTUAL-RUNTIME_initscript = ""

# Package management
PACKAGE_CLASSES = "package_rpm"
INHERIT += "rm_work"

# Image
IMAGE_CLASSES += "image_types"
IMAGE_FSTYPES = "ext4"
IMAGE_BOOT_FILES = "bzImage"
```

## 11. build/conf/bblayers.conf

```bitbake
# BBLayers configuration for MTS Core Router

BBLAYERS ?= " \
  ${TOPDIR}/poky/meta \
  ${TOPDIR}/poky/meta-poky \
  ${TOPDIR}/poky/meta-yocto-bsp \
  ${TOPDIR}/meta-openembedded/meta-oe \
  ${TOPDIR}/meta-openembedded/meta-networking \
  ${TOPDIR}/meta-intel \
  ${TOPDIR}/meta-mts \
"

BBLAYERS_CONF_VERSION = "2"
```
