# MTS-MC-5000 — Yocto + K3s Layer

## 1. Структура meta-mts-mobile

```
meta-mts-mobile/
├── conf/
│   ├── layer.conf
│   ├── machine/
│   │   └── mts-mc5000.conf
│   └── distro/
│       └── mts-mobile.conf
├── recipes-kernel/
│   ├── linux/
│   │   └── linux-mts_6.6.bb
│   └── dpdk/
│       └── dpdk-mts_23.11.bb
├── recipes-containers/
│   ├── k3s/
│   │   └── k3s_1.28.bb
│   ├── mts-upf/
│   │   └── mts-upf_1.0.bb
│   ├── mts-smf/
│   │   └── mts-smf_1.0.bb
│   ├── mts-amf/
│   │   └── mts-amf_1.0.bb
│   └── mts-pcf/
│       └── mts-pcf_1.0.bb
└── recipes-networking/
    ├── frrouting/
    │   └── frrouting_9.0.bb
    └── mts-pfcp/
        └── mts-pfcp_1.0.bb
```

## 2. layer.conf

```bitbake
BBPATH .= ":${LAYERDIR}"
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
            ${LAYERDIR}/recipes-*/*/*.bbappend"
BBFILE_COLLECTIONS += "meta-mts-mobile"
BBFILE_PATTERN_meta-mts-mobile = "^${LAYERDIR}/"
BBFILE_PRIORITY_meta-mts-mobile = "61"
LAYERSERIES_COMPAT_meta-mts-mobile = "kirkstone morty"
```

## 3. machine/mts-mc5000.conf

```bitbake
MACHINE = "mts-mc5000"
TUNE_FEATURES = "m64 armv8-a crc"
TARGET_FPU = ""
TARGET_ARCH = "aarch64"
TARGET_OS = "linux"
KERNEL_CLASSES = "kernel-image"
KERNEL_IMAGE_TYPE = "Image"
KERNEL_DEVICETREE = "mts-mc5000.dtb"
IMAGE_FSTYPES = "ext4"
IMAGE_BOOT_FILES = "Image mts-mc5000.dtb"
PACKAGE_CLASSES = "package_rpm"
INHERIT += "rm_work"
DISTRO_FEATURES_append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
DISTRO_FEATURES_BACKFILL_CONSIDERED = "sysvinit"
VIRTUAL-RUNTIME_initscript = ""
```

## 4. recipes-containers/k3s/k3s_1.28.bb

```bitbake
SUMMARY = "K3s for MTS Mobile Core"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=XXXX"

SRC_URI = "git://github.com/k3s-io/k3s.git;branch=v1.28"

S = "${WORKDIR}/git"

DEPENDS = "containerd runc"

do_configure() {
    make configure
}

do_compile() {
    make
}

do_install() {
    install -d ${D}/usr/local/bin
    install -m 0755 k3s ${D}/usr/local/bin/
    install -d ${D}/etc/systemd/system
    install -m 0644 files/k3s.service ${D}/etc/systemd/system/
}