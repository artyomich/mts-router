# Архитектура маршрутизаторов МТС — Обзор

## 1. Текущая инфраструктура МТС (анализ открытых данных)

МТС — крупнейший оператор связи России. Использует маршрутизаторы нескольких типов:

### 1.1 Дата-центры (MWS — МТС Web Services)
- Магистральные маршрутизаторы Cisco ASR 9000 / Nexus 9000
- Huawei NE40E / NE8000 для магистральных каналов
- Juniper MX-Series для BGP-транзита
- Arista 7050/7060 для spine-слоя
- Требуется: 100G/400G порты, BGP/ISIS, MPLS, SRv6

### 1.2 Сотовая сеть (4G LTE / 5G NR)
- Ядро EPC/5GC: маршрутизаторы для S1-U, N3 (user plane)
- Aggregation layer: Cisco ISR 4000 / Juniper EX
- BBU/eNodeB backhaul: MPLS-TP, PWE3
- Требуется: low-latency, GTP-U, QoS, RedCap support

### 1.3 Проводной доступ (FTTH/FTTB)
- GPON OLT: Huawei HG8245, ZTE C320, Nokia G-2426G
- Ethernet-маршрутизаторы для FTTB
- Требуется: GPON/EPON, VLAN, PPPoE, TR-069

### 1.4 Корпоративный сегмент (B2B)
- Маршрутизаторы для выделенных каналов
- SD-WAN, MPLS, IPsec
- Требуется: high availability, multiple WAN

### 1.5 Домашний сегмент (B2C)
- GPON/EPON ONU/ONT терминалы
- WiFi 6/7, VoIP, IPTV
- Требуется: compact, low-cost, TR-069/OMCI

---

## 2. Целевая архитектура маршрутизаторов

### 2.1 Core Router (MTS-CR-9000)
- Чип: Intel Tofino 2 (BSP) + Broadcom TomTom
- ОС: Yocto-based, DPDK/SPDK
- Порты: 64x 400G, 128x 100G
- Функции: BGP-4/6, MPLS, SRv6, segment routing

### 2.2 Mobile Core (MTS-MC-5000)
- Чип: Marvell ThunderX3 / Broadcom Trident 4
- ОС: Nephos / Yocto + Kubernetes
- Порты: 32x 100G, 64x 25G
- Функции: GTP-U, PFCP, UPF, 5G core

### 2.3 Mobile Backhaul (MTS-MB-3000)
- Чип: NXP S32G / Marvell 88Q5242
- ОС: Buildroot + DPDK
- Порты: 8x 10G, 24x 1G
- Функции: MPLS-TP, PWE3, SyncE, 1588v2

### 2.4 OLT GPON (MTS-OLT-2000)
- Чип: Intel Tofino 2 (forwarding) + Ralink/Realtek (GPON)
- ОС: OpenWrt / Yocto
- Порты: 16x 10G uplink, 192x GPON
- Функции: GPON/EPON, OMCI, TR-069

### 2.5 Enterprise Router (MTS-ER-1000)
- Чип: Broadcom TomTom / NXP S32G
- ОС: OpenWrt / VyOS
- Порты: 4x 10G, 8x 1G, WAN
- Функции: SD-WAN, MPLS, IPsec, BGP

### 2.6 Residential Gateway (MTS-RG-500)
- Чип: MediaTek MT7981 / Realtek RTL960x
- ОС: OpenWrt / Buildroot
- Порты: 1x GPON, 4x GE, WiFi 6
- Функции: GPON ONU, WiFi 6, VoIP, IPTV

---

## 3. Выбор ОС Linux

### 3.1 Core Router (MTS-CR-9000)
**Выбор: Yocto + DPDK + SPDK**
- Полная кастомизация под hardware
- Минимальный footprint
- DPDK для high-performance forwarding
- SPDK для NVMe storage management
- **Альтернатива: Nephos (NVIDIA)** — специализированная для network

### 3.2 Mobile Core (MTS-MC-5000)
**Выбор: Yocto + Kubernetes (K3s)**
- Контейнеризация для UPF, SMF, AMF
- Kubernetes для orchestration
- DPDK для user plane
- **Альтернатива: Linux Foundation's LF Edge — edgeX**

### 3.3 Mobile Backhaul (MTS-MB-3000)
**Выбор: Buildroot + DPDK**
- Минимальный образ для embedded
- Быстрый boot
- DPDK для packet processing
- **Альтернатива: OpenWrt** — если нужен WiFi backhaul

### 3.4 OLT GPON (MTS-OLT-2000)
**Выбор: OpenWrt**
- GPON драйверы уже есть
- TR-069, OMCI поддержка
- Веб-интерфейс
- **Альтернатива: Yocto** — если нужна кастомизация

### 3.5 Enterprise Router (MTS-ER-1000)
**Выбор: OpenWrt + FRRouting**
- FRRouting: BGP, OSPF, ISIS, MPLS
- OpenWrt: стабильная base
- **Альтернатива: VyOS** — если нужна enterprise CLI

### 3.6 Residential Gateway (MTS-RG-500)
**Выбор: OpenWrt**
- GPON драйверы
- WiFi management
- TR-069
- **Альтернатива: Buildroot** — для минимального образа

---

## 4. Выбор чипов

### 4.1 Core Router
| Компонент | Чип | Обоснование |
|-----------|-----|-------------|
| Forwarding | Intel Tofino 2 | P4-programmable, 100% programmable pipeline |
| CPU | AMD EPYC 7003 (Rome) | x86_64, high core count |
| Memory | DDR5 ECC | 512 GB+ |
| Storage | NVMe SSD | 4x 3.8 TB |
| ASIC | Broadcom TomTom | Backup для forwarding |

### 4.2 Mobile Core
| Компонент | Чип | Обоснование |
|-----------|-----|-------------|
| Forwarding | Marvell ThunderX3 | ARM64, high throughput |
| CPU | AMD EPYC 7002 | x86_64 |
| Memory | DDR4 ECC | 256 GB+ |
| Storage | NVMe SSD | 2x 3.8 TB |

### 4.3 Mobile Backhaul
| Компонент | Чип | Обоснование |
|-----------|-----|-------------|
| SoC | NXP S32G3 | ARM Cortex-A53/A72, automotive-grade |
| Switch | Marvell 88Q5242 | 5-port 10G |
| Memory | DDR4 | 8 GB+ |

### 4.4 OLT GPON
| Компонент | Чип | Обоснование |
|-----------|-----|-------------|
| Forwarding | Intel Tofino 2 | P4-programmable |
| CPU | AMD EPYC 7002 | x86_64 |
| GPON | Realtek RTL960x / Ralink RT5350 | GPON ONT/OLT |
| Memory | DDR4 | 32 GB+ |

### 4.5 Enterprise Router
| Компонент | Чип | Обоснование |
|-----------|-----|-------------|
| SoC | NXP S32G | ARM Cortex-A53/A72 |
| Switch | Broadcom TomTom | 10G/1G ports |
| Memory | DDR4 | 8 GB+ |

### 4.6 Residential Gateway
| Компонент | Чип | Обоснование |
|-----------|-----|-------------|
| SoC | MediaTek MT7981 (Filco) | ARM Cortex-A53, WiFi 6 integrated |
| GPON | Realtek RTL960x | GPON PHY |
| Memory | DDR3 | 512 MB+ |
| Storage | NAND Flash | 256 MB+ |

---

## 5. Рекомендации по открытым ОС

| Тип | ОС | Лицензия | Коммерческая доступность |
|-----|-----|----------|-------------------------|
| Core Router | Yocto Project | GPL-2.0 | ✓ Свободна |
| Mobile Core | Yocto + K3s | GPL-2.0 / Apache-2.0 | ✓ Свободна |
| Mobile Backhaul | Buildroot | GPL-2.0 | ✓ Свободна |
| OLT GPON | OpenWrt | GPL-2.0 | ✓ Свободна |
| Enterprise | OpenWrt + FRR | GPL-2.0 / BSD | ✓ Свободна |
| Residential | OpenWrt | GPL-2.0 | ✓ Свободна |
