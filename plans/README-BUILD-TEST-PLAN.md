# План реализации: Сборка образов и тестирование MTS Router

## Обзор

Детальный план реализации шести ожидающихся задач, охватывающий подготовку окружения, сборку образов (Yocto, Buildroot, OpenWrt), тестирование загрузки в QEMU, DPDK тесты на целевом оборудовании и финальные end-to-end интеграционные тесты.

## Реализованные компоненты

### Фаза 1: Подготовка окружения сборки

| Скрипт | Описание | Статус |
|--------|----------|--------|
| [`scripts/init-yocto-env.sh`](../../scripts/init-yocto-env.sh) | Инициализация Yocto/Poky окружения | Реализован |
| [`scripts/init-buildroot.sh`](../../scripts/init-buildroot.sh) | Инициализация Buildroot окружения | Реализован |
| [`scripts/init-openwrt-sdk.sh`](../../scripts/init-openwrt-sdk.sh) | Инициализация OpenWrt SDK с target profiles | Реализован |
| [`.github/actions/setup-build-env/action.yml`](../../.github/actions/setup-build-env/action.yml) | GitHub Action с кэшированием для CI/CD | Реализован |

### Фаза 2: Сборка образов

| Скрипт | Описание | Статус |
|--------|----------|--------|
| [`scripts/build-yocto-image.sh`](../../scripts/build-yocto-image.sh) | Оркестратор сборки с валидацией артефактов | Обновлен |
| [`scripts/build-mobile-backhaul.sh`](../../scripts/build-mobile-backhaul.sh) | Сборка MB-3000 (Buildroot) | Требуется обновление |
| [`scripts/build-olt-gpon.sh`](../../scripts/build-olt-gpon.sh) | Сборка OLT-2000 (OpenWrt) | Требуется обновление |
| [`scripts/build-enterprise.sh`](../../scripts/build-enterprise.sh) | Сборка ER-1000 (OpenWrt) | Требуется обновление |
| [`scripts/build-residential.sh`](../../scripts/build-residential.sh) | Сборка RG-500 (OpenWrt) | Требуется обновление |

### Фаза 3: QEMU тестирование

| Скрипт | Описание | Статус |
|--------|----------|--------|
| [`scripts/qemu/setup.sh`](../../scripts/qemu/setup.sh) | Настройка QEMU окружения и network tap | Реализован |
| [`scripts/qemu/run-test.sh`](../../scripts/qemu/run-test.sh) | Запуск单个 QEMU boot теста | Реализован |
| [`scripts/qemu/test-all.sh`](../../scripts/qemu/test-all.sh) | Запуск всех QEMU тестов с отчетом | Реализован |

### Фаза 4: DPDK тесты

| Скрипт | Описание | Статус |
|--------|----------|--------|
| [`scripts/dpdk/setup.sh`](../../scripts/dpdk/setup.sh) | Настройка hugepages и vfio-pci binding | Реализован |
| [`scripts/dpdk/run-benchmark.sh`](../../scripts/dpdk/run-benchmark.sh) | DPDK test-pmd benchmark с разными packet sizes | Реализован |

### Фаза 5: Интеграционные тесты

| Файл | Описание | Статус |
|------|----------|--------|
| [`scripts/integration/topology.yaml`](../../scripts/integration/topology.yaml) | Описание сетевой топологии (6 устройств, 7 сценариев) | Реализован |
| [`scripts/integration/run-tests.sh`](../../scripts/integration/run-tests.sh) | Запуск интеграционных тестов | Реализован |
| [`.github/workflows/build-test.yml`](../../.github/workflows/build-test.yml) | Полный CI/CD pipeline (5 фаз) | Реализован |

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

## Порядок выполнения

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

## Критерии завершения проекта

1. Все 6 образов собраны и артефакты доступны в `build/images/`
2. Все 6 образов проходят QEMU boot tests на x86_64 хосте
3. DPDK benchmarks выполнены на hardware для всех поддерживаемых устройств
4. Интеграционные тесты показывают 100% success rate
5. Все результаты задокументированы в `build/test-results/`
6. CI/CD pipeline проходит полностью в GitHub Actions

## Риски и способы минимизации

| Риск | Вероятность | Влияние | Митигация |
|------|-------------|---------|-----------|
| Yocto сборка падает из-за missing recipes | Высокая | Критический | Dry-run тестирование перед основной сборкой |
| Buildroot defconfig не совместим | Средняя | Высокий | Тестирование на clean buildroot tree |
| OpenWrt SDK не поддерживает target | Низкая | Высокий | Использование trunk/snapshot для свежих target |
| QEMU не может эмулировать ASIC | Средняя | Средний | Валидация только kernel boot и networking stack |
| Отсутствие DPDK hardware для тестов | Высокая | Критический | Использование QEMU with virtio-net как fallback |
| Интеграционные тесты требуют физическую топологию | Средняя | Высокий | Docker Compose для эмуляции сетевой топологии |

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
│   ├── build-cr9000.log
│   ├── build-mc5000.log
│   ├── build-mb3000.log
│   ├── build-olt2000.log
│   ├── build-er1000.log
│   └── build-rg500.log
└── test-results/
    ├── qemu/
    │   ├── results.json
    │   └── {aarch64,mips64el,arm}/boot-*.log
    ├── dpdk/
    │   ├── {cr9000,mc5000,mb3000,er1000}-dpdk-results.json
    │   └── summary.json
    └── integration/
        ├── summary.json
        └── {bgp_e2e, mpls, srv6, ptp, tr069, voip, iptv}.json
```
