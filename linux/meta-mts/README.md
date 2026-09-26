# meta-mts — Yocto Layer for MTS Router Devices

## Обзор

`meta-mts` — кастомный слой для Yocto Project, содержащий board support,
kernel recipes, image recipes и классы для всех устройств линейки МТС Роутер.

## Структура

```
meta-mts/
├── conf/
│   ├── layer.conf                    # Конфигурация слоя
│   └── machine/
│       ├── mts-cr9000.conf           # Core Router конфигурация
│       ├── mts-mc5000.conf           # Mobile Core конфигурация
│       ├── mts-mb3000.conf           # Mobile Backhaul конфигурация
│       ├── mts-olt2000.conf          # OLT GPON конфигурация
│       ├── mts-er1000.conf           # Enterprise Router конфигурация
│       └── mts-rg500.conf            # Residential Gateway конфигурация
├── classes/
│   └── mts-board.bbclass             # Базовый класс для всех board'ов
├── recipes-kernel/
│   ├── linux/
│   │   ├── mts-kernel_%.bbappend     # Шаблоны для kernel
│   │   ├── mts-kernel-tofino2.cfg    # Kernel config для Tofino 2
│   │   ├── mts-kernel-thunderx3.cfg  # Kernel config для ThunderX3
│   │   ├── mts-kernel-s32g3.cfg      # Kernel config для S32G3
│   │   ├── mts-kernel-mt7981.cfg     # Kernel config для MT7981
│   │   └── mts-kernel-rtl960x.cfg    # Kernel config для RTL960x
├── recipes-core/
│   ├── images/
│   │   ├── mts-core-router-image.bbappend
│   │   ├── mts-mobile-core-image.bbappend
│   │   ├── mts-mobile-backhaul-image.bbappend
│   │   ├── mts-olt-gpon-image.bbappend
│   │   ├── mts-enterprise-image.bbappend
│   │   └── mts-residential-image.bbappend
├── recipes-support/
│   ├── dpdk/
│   │   └── dpdk_23.11.bbappend       # DPDK для всех устройств
│   ├── frrouting/
│   │   └── frrouting_9.0.bbappend    # FRRouting для всех устройств
│   ├── mts-drivers/
│   │   ├── mts-tofino2-driver_1.0.bb
│   │   ├── mts-thunderx3-driver_1.0.bb
│   │   ├── mts-s32g3-driver_1.0.bb
│   │   ├── mts-rtl960x-driver_1.0.bb
│   │   └── mts-mt7981-driver_1.0.bb
│   ├── mts-api/
│   │   └── mts-api_1.0.bb            # MTS Router API server
│   ├── mts-sdk/
│   │   └── mts-sdk_1.0.bb            # MTS Router SDK
│   └── cwmp/
│       └── cwmpd_1.0.bb              # TR-069 client
├── recipes-bsp/
│   └── u-boot/
│       ├── u-boot-mts-cr9000.bbappend
│       ├── u-boot-mts-mc5000.bbappend
│       ├── u-boot-mts-mb3000.bbappend
│       ├── u-boot-mts-olt2000.bbappend
│       ├── u-boot-mts-er1000.bbappend
│       └── u-boot-mts-rg500.bbappend
└── docs/
    └── BUILD.md                      # Инструкция по сборке
```

## Поддерживаемые устройства

### MTS-CR-9000 (Core Router)
- **CPU:** AMD EPYC 7003 (Rome)
- **ASIC:** Intel Tofino 2
- **OS:** Yocto (Linux)
- **Kernel:** 6.6 LTS (custom)
- **Userspace:** DPDK 23.11, SPDK 23.11, FRRouting 9.0+

### MTS-MC-5000 (Mobile Core)
- **CPU:** Marvell ThunderX3 + AMD EPYC 7002
- **OS:** Yocto + K3s (Kubernetes)
- **Kernel:** 6.6 LTS (custom)
- **Userspace:** DPDK 23.11

### MTS-MB-3000 (Mobile Backhaul)
- **CPU:** NXP S32G3 + Marvell 88Q5242
- **OS:** Buildroot (Linux)
- **Kernel:** 6.6 LTS (custom)
- **Userspace:** DPDK 23.11, linuxptp

### MTS-OLT-2000 (OLT GPON)
- **CPU:** AMD EPYC 7002
- **ASIC:** Intel Tofino 2
- **GPON PHY:** Realtek RTL960x
- **OS:** OpenWrt
- **Kernel:** 6.6 LTS (custom)

### MTS-ER-1000 (Enterprise Router)
- **CPU:** NXP S32G3 + Broadcom TomTom
- **OS:** OpenWrt
- **Kernel:** 6.6 LTS (custom)
- **Userspace:** FRRouting 9.0+, strongSwan

### MTS-RG-500 (Residential Gateway)
- **CPU:** MediaTek MT7981
- **GPON PHY:** Realtek RTL960x
- **WiFi:** WiFi 6 (MT76)
- **OS:** OpenWrt
- **Kernel:** 6.6 LTS (custom)

## Сборка

### Подготовка окружения

```bash
# Клонирование источников
git clone https://git.yoctoproject.org/poky
git clone https://git.yoctoproject.org/meta-openembedded
git clone https://git.yoctoproject.org/meta-virtualization
git clone https://github.com/meta-container/k3s.git
git clone <meta-mts-repo>

# Добавление слоев
bitbake-layers add-layer meta-openembedded/meta-filesystems
bitbake-layers add-layer meta-openembedded/meta-networking
bitbake-layers add-layer meta-openembedded/meta-python
bitbake-layers add-layer meta-virtualization
bitbake-layers add-layer k3s
bitbake-layers add-layer meta-mts
```

### Сборка для Core Router

```bash
# Установка окружения
source oe-init-build-env build-cr9000

# Конфигурация
cp meta-mts/conf/machine/mts-cr9000.conf conf/local.conf
echo 'MACHINE = "mts-cr9000"' >> conf/local.conf

# Сборка
bitbake mts-core-router-image
```

### Сборка для Mobile Core

```bash
source oe-init-build-env build-mc5000
echo 'MACHINE = "mts-mc5000"' >> conf/local.conf
bitbake mts-mobile-core-image
```

### Сборка для Mobile Backhaul

```bash
source oe-init-build-env build-mb3000
echo 'MACHINE = "mts-mb3000"' >> conf/local.conf
bitbake mts-mobile-backhaul-image
```

### Сборка для OLT GPON

```bash
source oe-init-build-env build-olt2000
echo 'MACHINE = "mts-olt2000"' >> conf/local.conf
bitbake mts-olt-gpon-image
```

### Сборка для Enterprise Router

```bash
source oe-init-build-env build-er1000
echo 'MACHINE = "mts-er1000"' >> conf/local.conf
bitbake mts-enterprise-image
```

### Сборка для Residential Gateway

```bash
source oe-init-build-env build-rg500
echo 'MACHINE = "mts-rg500"' >> conf/local.conf
bitbake mts-residential-image
```

## Kernel Configurations

### Tofino 2 (Core Router)
```
CONFIG_INTEL_TOFINO2=y
CONFIG_INTEL_P4RT=y
CONFIG_DPDK=y
CONFIG_DPDK_MXLIO=y
CONFIG_MPLS=y
CONFIG_SEGMENT_ROUTING=y
```

### ThunderX3 (Mobile Core)
```
CONFIG_MARVELL_THUNDERX3=y
CONFIG_DPDK_MVNETA=y
CONFIG_CXL=y
CONFIG_5GC_UPF=y
CONFIG_K3S=y
```

### S32G3 (Backhaul/Enterprise)
```
CONFIG_NXP_S32G3=y
CONFIG_DPDK_NXP=y
CONFIG_PTP_1588_CLOCK=y
CONFIG_SYNC_E=y
CONFIG_IPSEC=y
```

### MT7981 (Residential Gateway)
```
CONFIG_MEDIATEK_MT7981=y
CONFIG_MEDIATEK_MT76=y
CONFIG_GPON_RTL960X=y
CONFIG_VAPOR=y
```

## CI/CD Integration

```bash
# Запуск сборки для всех устройств
./scripts/build-all.sh

# Запуск для конкретного устройства
./scripts/build-core-router.sh
./scripts/build-mobile-core.sh
./scripts/build-mobile-backhaul.sh
./scripts/build-olt-gpon.sh
./scripts/build-enterprise.sh
./scripts/build-residential.sh
```

## Лицензии

- Yocto Project: GPL-2.0
- DPDK: Apache-2.0
- FRRouting: GPL-2.0
- OpenWrt: GPL-2.0
- Собственные драйверы: PROPRIETARY
