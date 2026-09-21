# Анализ чипов для маршрутизаторов МТС

## 1. Intel Tofino 2 (BSP — Bitstream Programmable)

### Характеристики
- **Архитектура**: P4-programmable programmable pipeline
- **Порты**: до 128x 100G или 32x 400G
- **Packet Buffer**: 12 MB
- **Forwarding**: до 2.4 Tbps
- **Конфигурация**: перепрограммируемый ASIC

### Преимущества для МТС
- Полная programmability через P4
- Поддержка SRv6, segment routing
- Интеграция с OpenFlow, P4Runtime
- Низкая задержка (< 1 μs)
- Поддержка PFC, ECN, TCAM-free

### Недостатки
- Высокая стоимость (до $50K за чип)
- Требует лицензирования от Intel
- Сложная разработка драйверов

### Применение
- **MTS-CR-9000** (Core Router)
- **MTS-OLT-2000** (OLT GPON) — для forwarding

---

## 2. Broadcom TomTom

### Характеристики
- **Архитектура**: ASIC для маршрутизации
- **Порты**: до 64x 100G
- **Forwarding**: до 1.8 Tbps
- **Конфигурация**: fixed-function с программным программированием

### Преимущества для МТС
- Зрелая платформа
- Широкая поддержка протоколов
- Относительно низкая стоимость
- Стабильные драйверы

### Недостатки
- Менее programmable чем Tofino
- Зависимость от Broadcom

### Применение
- **MTS-CR-9000** (backup для Tofino)
- **MTS-ER-1000** (Enterprise Router)

---

## 3. Marvell ThunderX3

### Характеристики
- **Архитектура**: ARM64 Neoverse V1
- **Ядра**: до 96 cores
- **Память**: CXL 2.0, DDR5
- **Интерфейсы**: PCIe 5.0, 112 Gbps

### Преимущества для МТС
- Высокая производительность на ядро
- ARM64 — открытая архитектура
- Отлично для virtualized UPF
- Энергоэффективность

### Недостатки
- Менее оптимизирован для packet processing чем x86
- Требует DPDK

### Применение
- **MTS-MC-5000** (Mobile Core)

---

## 4. NXP S32G3

### Характеристики
- **Архитектура**: ARM Cortex-A53/A72
- **Ядра**: до 8 cores
- **Частота**: до 2 GHz
- **Интерфейсы**: Ethernet, CAN, FlexRay, SPI

### Преимущества для МТС
- Automotive-grade — высокая надежность
- Низкое энергопотребление
- Широкая поддержка от NXP
- Отлично для edge devices

### Недостатки
- Ограниченная производительность для core
- Не подходит для high-throughput

### Применение
- **MTS-MB-3000** (Mobile Backhaul)
- **MTS-ER-1000** (Enterprise Router)

---

## 5. MediaTek MT7981 (Filco)

### Характеристики
- **Архитектура**: ARM Cortex-A53 quad-core
- **Частота**: до 2 GHz
- **WiFi**: WiFi 6 (802.11ax) integrated
- **Ethernet**: 1x 10/100/1000 MAC
- **GPON**: integrated GPON PHY

### Преимущества для МТС
- Интеграция WiFi 6 + GPON в одном чипе
- Низкая стоимость (~$10-15)
- Широко используется в ONU/ONT
- Хорошая поддержка OpenWrt

### Недостатки
- Ограниченная производительность
- Не подходит для core

### Применение
- **MTS-RG-500** (Residential Gateway)

---

## 6. Realtek RTL960x

### Характеристики
- **Архитектура**: ARM
- **GPON**: PHY + MAC integrated
- **Ethernet**: 4x GE
- **WiFi**: external (MT76xx)

### Преимущества для МТС
- Отлично для GPON ONU/ONT
- Низкая стоимость
- Широкая поддержка в отрасли

### Недостатки
- Закрытая прошивка от Realtek
- Ограниченная кастомизация

### Применение
- **MTS-RG-500** (GPON PHY)
- **MTS-OLT-2000** (GPON line cards)

---

## 7. AMD EPYC 7003 (Rome) / 7002 (Naples)

### Характеристики
- **Архитектура**: x86_64 Zen 2 / Zen 1
- **Ядра**: до 64 cores (7003) / 32 cores (7002)
- **Память**: DDR4 ECC (7002) / DDR5 ECC (7003)
- **PCIe**: 128 lanes PCIe 4.0 (7003) / PCIe 3.0 (7002)

### Преимущества для МТС
- Высокая производительность для virtualized functions
- Широкая поддержка Linux
- Отлично для VNF/CNF

### Недостатки
- Высокое энергопотребление
- Требует хорошей системы охлаждения

### Применение
- **MTS-CR-9000** (control plane)
- **MTS-MC-5000** (control plane)
- **MTS-OLT-2000** (control plane)

---

## 8. Сравнительная таблица

| Чип | Архитектура | Производительность | Стоимость | Применимость |
|-----|-------------|-------------------|-----------|--------------|
| Intel Tofino 2 | P4 ASIC | 2.4 Tbps | $50K+ | Core Router, OLT |
| Broadcom TomTom | ASIC | 1.8 Tbps | $20K+ | Core, Enterprise |
| Marvell ThunderX3 | ARM64 | 96 cores | $5K+ | Mobile Core |
| NXP S32G3 | ARM | 8 cores @ 2GHz | $50+ | Backhaul, Enterprise |
| MediaTek MT7981 | ARM | 4 cores @ 2GHz | $10-15 | Residential |
| Realtek RTL960x | ARM | GPON PHY | $5-10 | GPON ONU/ONT |
| AMD EPYC 7003 | x86_64 | 64 cores | $1K+ | Control Plane |

---

## 9. Рекомендации

### Для Core Router (MTS-CR-9000)
- **Основной**: Intel Tofino 2 (programmable)
- **Backup**: Broadcom TomTom (fixed-function)
- **CPU**: AMD EPYC 7003 (Rome)
- **ОС**: Yocto + DPDK

### Для Mobile Core (MTS-MC-5000)
- **Основной**: Marvell ThunderX3 (ARM64)
- **CPU**: AMD EPYC 7002 (Naples)
- **ОС**: Yocto + K3s (Kubernetes)

### Для Mobile Backhaul (MTS-MB-3000)
- **SoC**: NXP S32G3
- **Switch**: Marvell 88Q5242
- **ОС**: Buildroot + DPDK

### Для OLT GPON (MTS-OLT-2000)
- **Forwarding**: Intel Tofino 2
- **CPU**: AMD EPYC 7002
- **GPON**: Realtek RTL960x
- **ОС**: OpenWrt

### Для Enterprise Router (MTS-ER-1000)
- **SoC**: NXP S32G
- **Switch**: Broadcom TomTom
- **ОС**: OpenWrt + FRR

### Для Residential Gateway (MTS-RG-500)
- **SoC**: MediaTek MT7981 (Filco)
- **GPON**: Realtek RTL960x
- **ОС**: OpenWrt
