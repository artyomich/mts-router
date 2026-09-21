# Выбор Linux ОС для маршрутизаторов МТС

## 1. Обзор доступных открытых ОС Linux для коммерческого использования

### 1.1 Yocto Project

| Параметр | Значение |
|----------|----------|
| **Лицензия** | GPL-2.0 |
| **Коммерческая доступность** | ✓ Полностью свободна |
| **Тип** | Build system для embedded Linux |
| **Производитель** | Linux Foundation |
| **Документация** | https://docs.yoctoproject.org |
| **Поддержка** | Широкая (Poky, meta-oe, meta-networking) |
| **Архитектуры** | x86_64, ARM, ARM64, RISC-V |

**Плюсы:**
- Полная кастомизация образа
- Контроль всех компонентов
- Минимальный footprint
- Поддержка DPDK, SPDK
- Production-ready (используется Intel, NXP, AMD)
- Мета-слои для networking (meta-networking)

**Минусы:**
- Высокий порог входа
- Требуется опыт сборки

**Применение:** MTS-CR-9000, MTS-MC-5000

---

### 1.2 Buildroot

| Параметр | Значение |
|----------|----------|
| **Лицензия** | GPL-2.0 |
| **Коммерческая доступность** | ✓ Полностью свободна |
| **Тип** | Build system для embedded Linux |
| **Производитель** | Linux Foundation |
| **Документация** | https://buildroot.org/downloads/manual |
| **Поддержка** | Широкая |
| **Архитектуры** | x86, ARM, ARM64, MIPS, PowerPC |

**Плюсы:**
- Проще Yocto для embedded
- Быстрая сборка
- Минимальный footprint
- Широкая поддержка чипов

**Минусы:**
- Меньше мета-слоев для networking
- Нет пакетного менеджера

**Применение:** MTS-MB-3000 (Mobile Backhaul)

---

### 1.3 OpenWrt

| Параметр | Значение |
|----------|----------|
| **Лицензия** | GPL-2.0 |
| **Коммерческая доступность** | ✓ Полностью свободна |
| **Тип** | Embedded Linux для маршрутизаторов |
| **Производитель** | OpenWrt Foundation |
| **Документация** | https://openwrt.org/docs |
| **Поддержка** | Широкая (1000+ устройств) |
| **Архитектуры** | ARM, MIPS, x86, RISC-V |

**Плюсы:**
- Специализирован для маршрутизаторов
- Встроенные пакеты (FRRouting, strongSwan, etc.)
- GPON драйверы уже есть
- TR-069, OMCI support
- LuCI Web UI
- Широкая поддержка WiFi (MTK, Realtek, Atheros)

**Минусы:**
- Менее гибкий для custom kernel
- Ограничен для high-performance forwarding

**Применение:** MTS-OLT-2000, MTS-ER-1000, MTS-RG-500

---

### 1.4 VyOS

| Параметр | Значение |
|----------|----------|
| **Лицензия** | GPL-2.0 |
| **Коммерческая доступность** | ✓ Полностью свободна |
| **Тип** | Network OS на базе Debian |
| **Производитель** | VyOS community |
| **Документация** | https://docs.vyos.io |
| **Поддержка** | FRRouting, strongSwan, etc. |
| **Архитектуры** | x86_64, ARM64 |

**Плюсы:**
- Enterprise-grade CLI
- Полная поддержка BGP, OSPF, ISIS, MPLS
- IPsec, VRRP, LACP
- Production-ready

**Минусы:**
- Требует Debian base (больший footprint)
- Не подходит для embedded

**Применение:** MTS-ER-1000 (альтернатива OpenWrt)

---

### 1.5 Nephos (NVIDIA)

| Параметр | Значение |
|----------|----------|
| **Лицензия** | Apache-2.0 |
| **Коммерческая доступность** | ✓ Свободна |
| **Тип** | Container-native network OS |
| **Производитель** | NVIDIA |
| **Документация** | https://github.com/nvidia/nephos |
| **Поддержка** | Kubernetes, DPDK |
| **Архитектуры** | x86_64, ARM64 |

**Плюсы:**
- Container-native для CNF
- Отлично для 5GC
- Kubernetes orchestration

**Минусы:**
- Зависимость от NVIDIA GPU
- Менее зрелая

**Применение:** MTS-MC-5000 (альтернатива)

---

### 1.6 EdgeX Foundry (LF Edge)

| Параметр | Значение |
|----------|----------|
| **Лицензия** | Apache-2.0 |
| **Коммерческая доступность** | ✓ Свободна |
| **Тип** | Edge computing framework |
| **Производитель** | Linux Foundation |
| **Документация** | https://edgeX.foundry.foundation |
| **Поддержка** | IoT, edge devices |
| **Архитектуры** | x86_64, ARM64, ARM |

**Плюсы:**
- Edge computing для IoT
- Microservices architecture

**Минусы:**
- Не специализирован для networking
- Сложная архитектура

**Применение:** MTS-MC-5000 (альтернатива)

---

## 2. Итоговая рекомендация по ОС

| Устройство | ОС | Обоснование |
|------------|-----|-------------|
| MTS-CR-9000 | **Yocto + DPDK** | Максимальный контроль, high-performance |
| MTS-MC-5000 | **Yocto + K3s** | Container-native для 5GC NFs |
| MTS-MB-3000 | **Buildroot + DPDK** | Minimal embedded, fast boot |
| MTS-OLT-2000 | **OpenWrt** | GPON drivers, TR-069, OMCI |
| MTS-ER-1000 | **OpenWrt + FRR** | Networking-ready, mature |
| MTS-RG-500 | **OpenWrt** | WiFi, GPON, TR-069, LuCI |

---

## 3. Структура Yocto проектов

### 3.1 MTS-CR-9000 (Yocto)

```
mts-core-router/
├── meta-mts/
│   ├── conf/
│   │   ├── layer.conf
│   │   ├── machine/
│   │   │   └── mts-cr9000.conf
│   │   └── distro/
│   │       └── mts-core.conf
│   ├── recipes-core/
│   │   ├── init-system-helpers/
│   │   └── systemd/
│   ├── recipes-kernel/
│   │   ├── linux/
│   │   │   └── linux-mts_6.6.bb
│   │   └── dpdk/
│   │       └── dpdk-mts_23.11.bb
│   ├── recipes-graphics/
│   │   └── tofino/
│   │       └── tofino-fw/
│   │           └── tofino-fw_2.0.bb
│   └── recipes-networking/
│       ├── frrouting/
│       │   └── frrouting_9.0.bb
│       └── mts-forwarder/
│           └── mts-forwarder_1.0.bb
├── build/
│   └── conf/
│       ├── local.conf
│       └── bblayers.conf
├── poky/
├── meta-openembedded/
├── meta-networking/
└── meta-intel/
```

### 3.2 MTS-MC-5000 (Yocto + K3s)

```
mts-mobile-core/
├── meta-mts/
│   ├── conf/
│   │   └── layer.conf
│   ├── recipes-kernel/
│   │   └── linux/
│   │       └── linux-mts_6.6.bb
│   ├── recipes-containers/
│   │   └── k3s/
│   │       └── k3s_1.28.bb
│   └── recipes-networking/
│       ├── dpdk/
│       │   └── dpdk-mts_23.11.bb
│       └── mts-upf/
│           └── mts-upf_1.0.bb
├── build/
│   └── conf/
├── poky/
├── meta-openembedded/
└── meta-virtualization/
```

### 3.3 MTS-MB-3000 (Buildroot)

```
mts-mobile-backhaul/
├── board/mts/mb3000/
│   ├── bootargs
│   ├── cmdline
│   ├── post-build.sh
│   └── post-image.sh
├── configs/
│   └── mts_mb3000_defconfig
├── package/
│   ├── mts-bh-forwarder/
│   │   ├── Mts-bh-forwarder.mk
│   │   └── mts-bh-forwarder.conf
│   ├── mts-ptp/
│   │   ├── Mts-ptp.mk
│   │   └── mts-ptp.conf
│   └── mts-mpls-tp/
│       ├── Mts-mpls-tp.mk
│       └── mts-mpls-tp.conf
└── build/
```

### 3.4 MTS-OLT-2000 (OpenWrt)

```
mts-olt-gpon/
├── package/
│   ├── mts-olt/
│   │   ├── Makefile
│   │   ├── src/
│   │   │   ├── olt_core.c
│   │   │   ├── olt_gpon.c
│   │   │   └── olt_omci.c
│   │   └── files/
│   │       └── init.d/mts-olt
│   ├── rtl960x-driver/
│   │   ├── Makefile
│   │   └── src/
│   │       └── rtl960x_gpon.c
│   └── mts-tr069/
│       ├── Makefile
│       └── src/
│           └── cwmpd.c
├── target/linux/
│   └── mts-olt-gpon/
│       ├── files/
│       ├── image/
│       ├── mts-olt-gpon.dts
│       └── patches-6.6/
├── configs/
│   └── mts-olt-gpon.config
├── feeds/
│   └── mts/
│       └── packages/
└── build/
```

### 3.5 MTS-ER-1000 (OpenWrt)

```
mts-enterprise-router/
├── package/
│   ├── mts-sdwan/
│   │   ├── Makefile
│   │   └── src/
│   │       └── sdwan_engine.c
│   └── mts-er/
│       ├── Makefile
│       └── src/
│           └── er_core.c
├── target/linux/
│   └── mts-er/
│       ├── files/
│       ├── image/
│       ├── mts-er.dts
│       └── patches-6.6/
├── configs/
│   └── mts-er.config
├── feeds/
│   └── mts/
│       └── packages/
└── build/
```

### 3.6 MTS-RG-500 (OpenWrt)

```
mts-residential-gateway/
├── package/
│   ├── mts-rg/
│   │   ├── Makefile
│   │   └── src/
│   │       └── rg_core.c
│   ├── mt7981-driver/
│   │   ├── Makefile
│   │   └── src/
│   │       └── mt7981_wifi.c
│   └── mts-voip/
│       ├── Makefile
│       └── src/
│           └── asterisk_mts.c
├── target/linux/
│   └── mts-rg/
│       ├── files/
│       ├── image/
│       ├── mts-rg.dts
│       └── patches-6.6/
├── configs/
│   └── mts-rg.config
├── feeds/
│   └── mts/
│       └── packages/
└── build/
```

---

## 4. Сравнительная таблица ОС

| ОС | Лицензия | Размер образа | Производительность | Сложность | Применимость |
|----|----------|---------------|-------------------|-----------|-------------|
| Yocto | GPL-2.0 | 50-200 MB | Высокая | Высокая | Core, Mobile Core |
| Buildroot | GPL-2.0 | 10-50 MB | Высокая | Средняя | Backhaul |
| OpenWrt | GPL-2.0 | 30-100 MB | Средняя | Низкая | OLT, Enterprise, RG |
| VyOS | GPL-2.0 | 500+ MB | Средняя | Низкая | Enterprise |
| Nephos | Apache-2.0 | 200-500 MB | Высокая | Высокая | Mobile Core |
| EdgeX | Apache-2.0 | 300-800 MB | Средняя | Высокая | Mobile Core |

---

## 5. Checklist для разработки

- [ ] Создать meta-mts слой для Yocto
- [ ] Создать board support для каждого устройства
- [ ] Написать kernel recipes для каждого чипа
- [ ] Написать package recipes для каждого ПО
- [ ] Настроить build для каждого устройства
- [ ] Протестировать boot sequence
- [ ] Протестировать networking stack
- [ ] Протестировать DPDK integration
- [ ] Протестировать API
- [ ] Добавить CI/CD для сборки
