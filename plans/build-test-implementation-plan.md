# План реализации: Сборка образов и тестирование MTS Router

## Обзор

Детальный план реализации шести ожидающихся задач, охватывающий подготовку окружения, сборку образов (Yocto, Buildroot, OpenWrt), тестирование загрузки в QEMU, DPDK тесты на целевом оборудовании и финальные end-to-end интеграционные тесты.

## Архитектура CI/CD Pipeline

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    GitHub Actions Workflow                               │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐                │
│  │  Phase 1    │    │  Phase 2    │    │  Phase 3    │                │
│  │  Preparation│───►│   Building  │───►│   Testing   │                │
│  └─────────────┘    └─────────────┘    └─────────────┘                │
│       │                   │                   │                        │
│       ▼                   ▼                   ▼                        │
│  Yocto env          All images           QEMU + Hardware              │
│  Buildroot tree     6 artifacts          Integration tests            │
│  OpenWrt SDK        Cached builds        DPDK validation              │
└─────────────────────────────────────────────────────────────────────────┘
```

## Зависимости между задачами

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        ЗАВИСИМОСТИ                                         │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  [P1] Yocto Environment ────────► [P2a] Core Router Image               │
│  [P1] Yocto Environment ────────► [P2b] Mobile Core Image             │
│  [P1] Buildroot Tree ───────────► [P2c] MB-3000 Image                   │
│  [P1] OpenWrt SDK ──────────────► [P2d] OLT-2000 Image                  │
│  [P1] OpenWrt SDK ──────────────► [P2e] ER-1000 Image                   │
│  [P1] OpenWrt SDK ──────────────► [P2f] RG-500 Image                    │
│                                                                          │
│  [P2a-f] Все образы ────────────► [P3] QEMU Boot Tests                  │
│  [P3] QEMU Passed ──────────────► [P4] DPDK Hardware Tests              │
│  [P3] QEMU Passed ──────────────► [P5] Integration Tests                │
│  [P4] DPDK Passed ──────────────► [P5] Integration Tests                │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Фаза 1: Подготовка окружения сборки

### Цель
Инициализировать все три системы сборки (Yocto, Buildroot, OpenWrt) в изолированных Docker контейнерах с кэшированием через GitHub Actions.

### Шаг 1.1: Инициализация Yocto окружения

**Действия:**
1. Создать Dockerfile с poky-репозиторием (release kirkstone или langdale)
2. Настроить bitbake с meta-openembedded, meta-virtualization, meta-mts
3. Создать script `scripts/init-yocto-env.sh` для автоматической инициализации

**Необходимые инструменты:**
- Docker (x86_64)
- poky (Yocto 3.4+ Kirkstone/Langdale)
- meta-openembedded (layer for networking packages)
- meta-virtualization (layer for K3s/Kubernetes support)
- meta-mts (custom layer из `linux/meta-mts/`)

**Конфигурация machine:**
```bash
# meta-mts/conf/machine/mts-cr9000.conf
MACHINE = "mts-cr9000"
SOC_FAMILY = "x86-64"
DISTRO = "mts-router"
PACKAGE_CLASSES = "package_rpm package_deb"
IMAGE_FSTYPES = "ext4 wic"
TARGET_ARCH = "x86_64"
```

**Ожидаемый артефакт:** Рабочее Yocto окружение с валидной конфигурацией для mts-cr9000 и mts-mc5000

**Критерий успеха:** `bitbake mts-core-router-image --dry-run` проходит без ошибок

### Шаг 1.2: Инициализация Buildroot окружения

**Действия:**
1. Клонировать tree Buildroot (release 2024.05+)
2. Скопировать `linux/buildroot/mts-s32g3-defconfig` в buildroot/output/
3. Настроить BR2_EXTERNAL для meta-mts пакетов

**Необходимые инструменты:**
- Buildroot 2024.05+
- Toolchain для S32G3 (aarch64-linux-gnu-gcc >= 12)
- meta-mts recipes для DPDK и PTP packages

**Ожидаемый артефакт:** Валидный defconfig для mts-mb3000 (S32G3)

**Критерий успеха:** `make mts-s32g3_defconfig && make` завершается созданием `images/uImage` и `images/rootfs.cpio.gz`

### Шаг 1.3: Инициализация OpenWrt SDK

**Действия:**
1. Создать `scripts/init-openwrt-sdk.sh` для клонирования и настройки OpenWrt snapshot/trunk
2. Подготовить target profiles для:
   - mts-olt2000 (ARM64, RTL960x + Tofino 2)
   - mts-er1000 (ARM64, S32G3 + TomTom)
   - mts-rg500 (MIPS/ARM, MT7981 + RTL960x)
3. Добавить meta-mts-openwrt layer в feeds.conf

**Необходимые инструменты:**
- OpenWrt SDK (trunk/snapshot)
- Toolchains: aarch64-linux-gnu, mipsel-linux-gnu
- DTS файлы из `*/linux/device-tree.dts`

**Ожидаемый артефакт:** Три валидных target profiles в OpenWrt

**Критерий успеха:** `make defconfig` для каждого target проходит без ошибок

### Шаг 1.4: GitHub Actions кэширование

**Действия:**
1. Создать `.github/actions/setup-build-env/action.yml`
2. Настроить кэширование:
   - Yocto sstate-cache (key: `yocto-sstate-${{ runner.os }}`)
   - Buildroot dl/ (key: `buildroot-dl-${{ runner.os }}`)
   - OpenWrt dl/ (key: `openwrt-dl-${{ runner.os }}`)
3. Добавить cache restore/save в каждый job

**Ожидаемый артефакт:** Workflow с кэшированием, сокращающим время сборки на 60%+

---

## Фаза 2: Сборка образов

### Цель
Собрать все 6 OS образов в изолированных GitHub Actions jobs с параллельным выполнением.

### Шаг 2.1: Сборка Yocto образов (mts-cr9000, mts-mc5000)

**Действия:**
1. Обновить `scripts/build-core-router.sh`:
   - Проверка окружения (oe-init-build-env)
   - Добавление meta-mts слоя
   - Запуск bitbake mts-core-router-image
   - Валидация выхода (kernel, rootfs, device-tree)
2. Обновить `scripts/build-mobile-core.sh`:
   - Добавление meta-kubernetes для K3s
   - Запуск bitbake mts-mobile-core-image
3. Добавить `scripts/verify-yocto-image.sh` для проверки артефактов

**Команды сборки:**
```bash
# Core Router
source poky/oe-init-build-env build/cr9000
bitbake-layers add-layer ../../linux/meta-mts
bitbake mts-core-router-image

# Mobile Core
source poky/oe-init-build-env build/mc5000
bitbake-layers add-layer ../../linux/meta-mts
bitbake mts-mobile-core-image
```

**Ожидаемые артефакты:**
- `build/tmp/deploy/images/mts-cr9000/` — kernel, dtb, rootfs
- `build/tmp/deploy/images/mts-mc5000/` — kernel, dtb, rootfs, k3s packages

**Критерий успеха:** Файлы `Image`, `*.dtb`, `mts-core-router-image.ext4` существуют и имеют размер > 0

### Шаг 2.2: Сборка Buildroot образа (mts-mb3000)

**Действия:**
1. Обновить `scripts/build-mobile-backhaul.sh`:
   - Настройка BR2_EXTERNAL
   - Запуск make для mts-s32g3_defconfig
   - Валидация выхода
2. Добавить проверку DPDK и PTP пакетов в rootfs

**Команды сборки:**
```bash
make mts-s32g3_defconfig BR2_EXTERNAL=../../linux/meta-mts
make
```

**Ожидаемые артефакты:**
- `output/images/uImage`
- `output/images/rootfs.cpio.gz`
- `output/images/sdcard.img` (если используется)

**Критерий успеха:** Все основные пакеты (DPDK, PTP daemon, MPLS tools) присутствуют в rootfs

### Шаг 2.3: Сборка OpenWrt образов (mts-olt2000, mts-er1000, mts-rg500)

**Действия:**
1. Обновить `scripts/build-olt-gpon.sh`, `scripts/build-enterprise.sh`, `scripts/build-residential.sh`
2. Каждый script:
   - Инициализирует OpenWrt SDK
   - Применяет custom target config
   - Запускает make
   - Валидирует образ
3. Добавить `scripts/verify-openwrt-image.sh`

**Команды сборки:**
```bash
# OLT GPON
make defconfig TARGET="armsr/armv10" SUBTARGET="generic"
# Apply custom kernel config and feeds
make -j$(nproc)

# Enterprise
make defconfig TARGET="freescale/armv7" SUBTARGET="generic"
make -j$(nproc)

# Residential
make defconfig TARGET="mediatek/mt7981" SUBTARGET="generic"
make -j$(nproc)
```

**Ожидаемые артефакты:**
- `bin/targets/*/` — sysupgrade.bin, factory.bin
- Пакеты: cwmp, asterisk, iptables, odhcpd

**Критерий успеха:** Образы sysupgrade.bin существуют и проходят `fwtool -I` проверку

---

## Фаза 3: Тестирование загрузки в QEMU

### Цель
Валидировать все 6 образов в QEMU эмуляции на x86_64 хосте перед тестированием на hardware.

### Шаг 3.1: Настройка QEMU окружения

**Действия:**
1. Создать `scripts/qemu/setup.sh`:
   - Установка QEMU (>= 8.0) с aarch64/ARM/mipsel targets
   - Загрузка kernel images для эмуляции
   - Настройка network tap для тестирования
2. Создать `scripts/qemu/run-test.sh` с параметрами:
   - `--image <path-to-image>`
   - `--arch <aarch64|mipsel|arm>`
   - `--timeout <seconds>`
   - `--verify <check-script>`

**Необходимые инструменты:**
- QEMU 8.0+ (qemu-system-aarch64, qemu-system-mips64el)
- Kernel images из Phase 2
- Rootfs images из Phase 2

### Шаг 3.2: Запуск QEMU тестов для каждого образа

**Действия:**
1. Создать `scripts/qemu/test-all.sh`:
   - Для каждого образа: запуск QEMU, проверка boot, проверка networking
   - Логирование в `build/test-results/qemu/`
   - Генерация отчета в JSON
2. Тестовые чекпоинты:
   - Boot completion (< 60s)
   - Network interface up (eth0)
   - SSH daemon responding
   - Critical services running (systemd/init)

**Команды тестирования:**
```bash
# AArch64 (MB-3000, MC-5000)
qemu-system-aarch64 -M virt -cpu cortex-a72 \
    -kernel uImage -append "console=ttyAMA0" \
    -drive file=rootfs.cpio.gz,format=raw \
    -netdev user,id=n1 -device virtio-net-device,netdev=n1 \
    -nographic -m 2048

# MIPS64 (OLT-2000)
qemu-system-mips64el -M malta -cpu MIPS64 \
    -kernel vmlinux-initrd.bin -append "console=ttyS0" \
    -drive file=squashfs.img,format=raw \
    -netdev user,id=n1 -device e1000,netdev=n1 \
    -nographic -m 1024

# ARM (ER-1000, RG-500)
qemu-system-arm -M versatilepb -cpu arm1176 \
    -kernel zImage -append "console=ttyAMA0" \
    -drive file=rootfs.ext4,format=raw \
    -netdev user,id=n1 -device rtl8139,netdev=n1 \
    -nographic -m 512
```

**Ожидаемые артефакты:**
- `build/test-results/qemu/{cr9000,mc5000,mb3000,olt2000,er1000,rg500}/boot.log`
- `build/test-results/qemu/results.json`

**Критерий успеха:** Все 6 образов загружаются и проходят базовые чекпоинты в QEMU

---

## Фаза 4: DPDK тесты на целевом оборудовании

### Цель
Валидировать DPDK forwarding performance на реальном hardware для каждого устройства.

### Шаг 4.1: Подготовка DPDK тестового окружения

**Действия:**
1. Создать `scripts/dpdk/setup.sh`:
   - Инсталляция DPDK на target device
   - Настройка hugepages
   - Настройка NIC bindings (vfio-pci)
2. Создать `scripts/dpdk/run-benchmark.sh`:
   - Test-pmd forwarding test
   - Packet size variants (64, 128, 512, 1518 bytes)
   - Duration: 60s per test
   - Measurement: Mpps, latency, CPU utilization

**Необходимое hardware:**
- MTS-CR-9000: Tofino 2 ASIC + 100G ports
- MTS-MC-5000: ThunderX3 + 100G ports
- MTS-MB-3000: S32G3 + 10G ports
- MTS-ER-1000: S32G3 + 10G ports

### Шаг 4.2: Запуск DPDK тестов

**Действия:**
1. Для каждого device запустить test-pmd:
```bash
# Configure hugepages
echo 2048 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

# Bind NIC to vfio-pci
modprobe vfio-pci
dpdk-devbind.py --bind=vfio-pci 0000:01:00.0

# Run test-pmd
testpmd -l 0-3 -n 4 -- -i --portmask=0x3 --rxdesc=1024 --txdesc=1024
testpmd> set fwd mac
testpmd> start
# Measure for 60s with traffic generator (pktgen/dpkt)
testpmd> stop
```

2. Собрать результаты в `build/test-results/dpdk/`:
   - `mts-cr9000-dpdk-results.json`
   - `mts-mc5000-dpdk-results.json`
   - `mts-mb3000-dpdk-results.json`
   - `mts-er1000-dpdk-results.json`

**Критерий успеха:**
- MTS-CR-9000: >= 50 Mpps at 64-byte (line rate on 100G)
- MTS-MC-5000: >= 50 Mpps at 64-byte
- MTS-MB-3000: >= 10 Mpps at 64-byte (line rate on 10G)
- MTS-ER-1000: >= 10 Mpps at 64-byte
- CPU utilization < 80% per core

---

## Фаза 5: End-to-end интеграционные тесты

### Цель
Валидировать взаимодействие всех устройств в сетевой топологии.

### Шаг 5.1: Настройка интеграционной топологии

**Действия:**
1. Создать `scripts/integration/topology.yaml`:
```yaml
topology:
  core_router:
    device: mts-cr9000
    ip: 10.0.0.1
    roles: [bgp, mpls, srv6]
  mobile_core:
    device: mts-mc5000
    ip: 10.0.0.2
    roles: [upf, smf]
  mobile_backhaul:
    device: mts-mb3000
    ip: 10.0.0.3
    roles: [mpls-tp, ptp]
  olt_gpon:
    device: mts-olt2000
    ip: 10.0.0.4
    roles: [ont-mgmt, tr069]
  enterprise:
    device: mts-er1000
    ip: 10.0.0.5
    roles: [sdwan, vip]
  residential:
    device: mts-rg500
    ip: 10.0.0.6
    roles: [dhcp, wifi, voip]
```

2. Настроить network bridge на хосте для соединения контейнеров/VM

### Шаг 5.2: Запуск интеграционных тестов

**Действия:**
1. Обновить `scripts/test-integration.sh`:
   - Развертывание всех устройств в топологии
   - Тестирование BGP peer adjacency
   - Тестирование MPLS label switching
   - Тестирование SRv6 policy forwarding
   - Тестирование PTP grandmaster/slave sync
   - Тестирование TR-069 provisioning
   - Тестирование IPTV multicast
   - Тестирование VoIP call setup
2. Добавить `scripts/integration/verify-all.sh` для финальной валидации

**Тестовые сценарии:**
```bash
# 1. BGP connectivity
bgpq4 -H -q AS65001 | ssh core-router "ip route show table bgp"

# 2. MPLS label switching
traceroute -m 10 10.0.10.1  # Should show MPLS labels

# 3. PTP sync
ptp4l -H  # Check offset < 1us

# 4. TR-069 provisioning
curl -u admin:password http://olt2000:7547/getparametername

# 5. VoIP call setup
asterisk -rx "sip peer show rg500"
asterisk -rx "channel originate SIP/rg500 extension 100"

# 6. IPTV multicast
igmpjoin 239.1.1.1 on eth1
iperf -u -B 239.1.1.1 -c 239.1.1.1 -t 30
```

**Ожидаемые артефакты:**
- `build/test-results/integration/{cr9000,mc5000,mb3000,olt2000,er1000,rg500}/`
- `build/test-results/integration/summary.json`

**Критерий успеха:**
- 100% BGP routes exchanged
- MPLS labels assigned and forwarding
- PTP sync < 1us offset
- TR-069 provisioning successful
- VoIP call established < 2s
- IPTV multicast streaming without packet loss

---

## Порядок выполнения и зависимости

```
Неделя 1-2: Фаза 1 (Подготовка окружения)
    ├── 1.1: Yocto env
    ├── 1.2: Buildroot tree
    ├── 1.3: OpenWrt SDK
    └── 1.4: GitHub Actions caching

Неделя 3-4: Фаза 2 (Сборка образов)
    ├── 2.1: Yocto images (cr9000, mc5000)
    ├── 2.2: Buildroot image (mb3000)
    └── 2.3: OpenWrt images (olt2000, er1000, rg500)

Неделя 5: Фаза 3 (QEMU тесты)
    ├── 3.1: QEMU setup
    └── 3.2: Boot tests for all 6 images

Неделя 6-7: Фаза 4 (DPDK тесты)
    ├── 4.1: DPDK env setup
    └── 4.2: Hardware benchmarks

Неделя 8: Фаза 5 (Интеграционные тесты)
    ├── 5.1: Topology setup
    └── 5.2: End-to-end validation
```

---

## Критерии завершения проекта

1. Все 6 образов собраны и артефакты доступны в `build/images/`
2. Все 6 образов проходят QEMU boot tests на x86_64 хосте
3. DPDK benchmarks выполнены на hardware для всех поддерживаемых устройств
4. Интеграционные тесты показывают 100% success rate
5. Все результаты задокументированы в `build/test-results/`
6. CI/CD pipeline проходит полностью в GitHub Actions

---

## Риски и способы минимизации

| Риск | Вероятность | Влияние | Митигация |
|------|-------------|---------|-----------|
| Yocto сборка падает из-за missing recipes | Высокая | Критический | Dry-run тестирование перед основной сборкой |
| Buildroot defconfig не совместим | Средняя | Высокий | Тестирование на clean buildroot tree |
| OpenWrt SDK не поддерживает target | Низкая | Высокий | Использование trunk/snapshot для свежих target |
| QEMU не может эмулировать ASIC | Средняя | Средний | Валидация только kernel boot и networking stack |
| Отсутствие DPDK hardware для тестов | Высокая | Критический | Использование QEMU with virtio-net как fallback |
| Интеграционные тесты требуют физическую топологию | Средняя | Высокий | Docker Compose для эмуляции сетевой топологии |

---

## Артефакты на выходе

```
build/
├── images/
│   ├── mts-cr9000/
│   │   ├── Image
│   │   ├── *.dtb
│   │   └── mts-core-router-image.ext4.wic.xz
│   ├── mts-mc5000/
│   │   ├── Image
│   │   ├── *.dtb
│   │   └── mts-mobile-core-image.ext4.wic.xz
│   ├── mts-mb3000/
│   │   ├── uImage
│   │   └── rootfs.cpio.gz
│   ├── mts-olt2000/
│   │   └── openwrt-armsr-armv8-generic-squashfs-combined.img.gz
│   ├── mts-er1000/
│   │   └── openwrt-freescale-armv8-generic-squashfs-combined.img.gz
│   └── mts-rg500/
│       └── openwrt-mediatek-mt7981-generic-squashfs-combined.img.gz
├── logs/
│   ├── build-core-router.log
│   ├── build-mobile-core.log
│   ├── build-mobile-backhaul.log
│   ├── build-olt-gpon.log
│   ├── build-enterprise.log
│   └── build-residential.log
└── test-results/
    ├── qemu/
    │   ├── results.json
    │   └── {cr9000,mc5000,mb3000,olt2000,er1000,rg500}/boot.log
    ├── dpdk/
    │   ├── {cr9000,mc5000,mb3000,er1000}-dpdk-results.json
    │   └── summary.json
    └── integration/
        ├── summary.json
        └── {cr9000,mc5000,mb3000,olt2000,er1000,rg500}/
```
