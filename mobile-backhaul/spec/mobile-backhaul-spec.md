# MTS-MB-3000 — Mobile Backhaul Router

## Спецификация маршрутизатора агрегации базовых станций

---

## 1. Назначение

MTS-MB-3000 — маршрутизатор для агрегации трафика от базовых станций (eNodeB/gNodeB)
в ядро сотовой сети МТС. Обеспечивает:
- Backhaul для 4G LTE eNodeB
- Fronthaul/Midhaul для 5G gNodeB
- MPLS-TP, PWE3 для симуляции Ethernet
- SyncE и 1588v2 для синхронизации
- Low-latency forwarding для fronthaul

---

## 2. Аппарная спецификация

### 2.1 Основная плата (Main Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **SoC** | NXP S32G3 | ARM Cortex-A53/A72, 8 cores |
| **Switch** | Marvell 88Q5242 | 5-port 10G |
| **Memory** | 8 GB DDR4 ECC | |
| **Storage** | 128 GB eMMC | Boot + OS |
| **Crypto** | NXP SEC (Hardware) | IPsec, TLS |
| **Power** | 150W AC/DC | Redundant 1+1 |
| **Dimensions** | 1RU | 442 x 445 x 44.5 mm |

### 2.2 Плата портов (Line Card)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Ports** | 8x 10G SFP+ | Uplink to core |
| **Ports** | 24x 1G SFP | Downlink to BBU |
| **Sync** | 1588v2, SyncE | Clock distribution |
| **Memory** | 1 GB DDR4 | Per card |

### 2.3 Вентиляция

| Параметр | Значение |
|----------|----------|
| **Airflow** | Front-to-back |
| **Fans** | 4x hot-swap fans |
| **Cooling** | Forced air |

---

## 3. Сетевые возможности

### 3.1 Протоколы

| Протокол | Версия | Поддержка |
|----------|--------|-----------|
| MPLS-TP | RFC 5327/5921 | ✓ |
| PWE3 | RFC 4448/4754 | ✓ |
| GTP-U | 14.x | ✓ |
| BGP-4 | — | ✓ |
| OSPF | — | ✓ |
| LACP | — | ✓ |
| 1588v2 | PTP | ✓ |
| SyncE | ITU-T G.8261 | ✓ |
| IEEE 101 | ITU-T G.8275 | ✓ |

### 3.2 QoS для сотовой трафик

| Параметр | Значение |
|----------|----------|
| **Classes** | 8 per port |
| **Scheduling** | WRR, Strict |
| **Policing** | Token bucket |
| **Marking** | DSCP, EXP |

### 3.3 Синхронизация

| Параметр | Значение |
|----------|----------|
| **1588v2** | Precision < 1 μs |
| **SyncE** | ITU-T G.8261 |
| **Holdover** | OCXO, < 1.5e-11 |
| **Grandmaster** | PTP Grandmaster mode |

### 3.4 HA

| Параметр | Значение |
|----------|----------|
| **Failover** | < 50 ms |
| **Redundancy** | 1+1 power |
| **NSF** | ✓ |

---

## 4. Программная платформа

### 4.1 Операционная система

| Параметр | Значение |
|----------|----------|
| **Base OS** | Buildroot (Linux) |
| **Kernel** | 6.6 LTS (custom) |
| **Userspace** | DPDK 23.11 |
| **Routing** | FRRouting 9.0+ |
| **Sync** | PTP (linuxptp) |
| **Management** | NETCONF, CLI |

### 4.2 DPDK-приложения

| Приложение | Назначение |
|------------|------------|
| `mts-bh-forwarder` | Backhaul packet forwarding |
| `mts-ptp` | PTP grandmaster/sync |
| `mts-mpls-tp` | MPLS-TP forwarding |
| `mts-ctrl` | Management plane |

### 4.3 YANG-модели

```
mts-mobile-backhaul/
├── mts-bh-sync.yang
├── mts-bh-mpls.yang
├── mts-bh-pwe3.yang
├── mts-bh-telemetry.yang
└── mts-bh-ctrl.yang
```

---

## 5. Требования к Linux-драйверам

### 5.1 NXP S32G3

| Драйвер | Назначение |
|---------|------------|
| `s32g-net` | Ethernet TX/RX |
| `s32g-sec` | Hardware crypto (IPsec) |
| `s32g-ptp` | PTP hardware timestamping |
| `s32g-sync` | SyncE management |

### 5.2 Marvell 88Q5242

| Драйвер | Назначение |
|---------|------------|
| `mv88q5242-phy` | PHY management |
| `mv88q5242-mdio` | MDIO bus |

### 5.3 DPDK PMD

| PMD | Назначение |
|-----|------------|
| `dpdk-nxp` | NXP S32G network |
| `dpdk-netvsc` | Virtual network |

---

## 6. Энергопотребление

| Режим | Потребление |
|-------|-------------|
| **Idle** | 60W |
| **Load** | 140W |
| **Peak** | 150W |

---

## 7. Физические параметры

| Параметр | Значение |
|----------|----------|
| **Размеры** | 442 x 445 x 44.5 mm (1RU) |
| **Вес** | 5 kg (max) |
| **Рабочая темп.** | -40°C ... +75°C |
| **Влажность** | 5% ... 95% non-condensing |
| **Шум** | < 45 dB(A) |
| **Protection** | IP30 |

---

## 8. API спецификация (REST)

### 8.1 Основные эндпоинты

```
GET    /api/v1/sync               — PTP sync state
PUT    /api/v1/sync/grandmaster   — Set grandmaster
GET    /api/v1/mpls-tp            — MPLS-TP state
POST   /api/v1/mpls-tp/pe         — Create pseudowire
GET    /api/v1/interfaces         — Interface list
GET    /api/v1/telemetry         — Telemetry
GET    /api/v1/health            — Health check
```

### 8.2 Пример запроса (создание pseudowire)

```json
{
  "method": "POST",
  "url": "/api/v1/mpls-tp/pe",
  "body": {
    "ingress_port": "eth1",
    "egress_port": "eth2",
    "pw_id": 1001,
    "encapsulation": "eth",
    "qos": "silver"
  }
}
```

---

## 9. Трассировка платы

### 9.1 Основные сигналы

| Сигнал | Тип | Скорость | Примечание |
|--------|-----|----------|------------|
| PCIe Gen3 x4 | SoC ↔ Switch | 8 GT/s | |
| DDR4 | SoC ↔ DDR4 | 2400 MT/s | |
| I2C | BMC ↔ All | 100 kHz | Management |
| SPI | BMC ↔ Flash | 50 MHz | Boot |
| PTP | All ports | 1588v2 | Sync |
| SyncE | All ports | 156.25 MHz | Clock |

### 9.2 PCB параметры

| Параметр | Значение |
|----------|----------|
| **Layers** | 8-layer |
| **Material** | FR-4 |
| **Impedance** | 50Ω single, 100Ω differential |
| **Copper** | 1 oz (outer), 0.5 oz (inner) |
| **Surface** | ENIG |

---

## 10. Checklist для разработки

- [ ] Проектирование main board (S32G3 + 88Q5242)
- [ ] Проектирование line card (10G/1G ports)
- [ ] Разработка драйверов S32G3
- [ ] Сборка Buildroot образа
- [ ] Интеграция DPDK
- [ ] Разработка MPLS-TP forwarding
- [ ] Разработка PTP grandmaster
- [ ] Разработка YANG моделей
- [ ] Разработка REST API
- [ ] Тестирование SyncE/1588v2
- [ ] Тестирование MPLS-TP
- [ ] Тестирование HA
- [ ] Сертификация
