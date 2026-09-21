# MTS-CR-9000 — Core Router

## Спецификация магистрального маршрутизатора для дата-центров МТС

---

## 1. Назначение

MTS-CR-9000 — магистральный маршрутизатор для дата-центров МТС (MWS), обеспечивающий:
- BGP-транзит между ЦОД
- MPLS/SRv6 маршрутизацию
- Высокую доступность (99.999%)
- Масштабирование до 100+ Tbps

---

## 2. Аппарная спецификация

### 2.1 Основная плата (Main Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Forwarding ASIC** | Intel Tofino 2 | P4-programmable, 2.4 Tbps |
| **Backup ASIC** | Broadcom TomTom | Fixed-function, 1.8 Tbps |
| **CPU** | AMD EPYC 7003 (Rome) | 32 cores, 64 threads, 2.5 GHz |
| **Memory** | 512 GB DDR5 ECC | 8x 64 GB RDIMM |
| **Storage** | 4x 3.8 TB NVMe SSD | RAID-10, U.2 |
| **Fabric** | 128 Gbps crossbar | Non-blocking |
| **Power** | 1200W AC/DC | Redundant 1+1 |
| **Dimensions** | 1RU | 442 x 445 x 44.5 mm |

### 2.2 Плата управления (Control Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **CPU** | AMD EPYC 7003 | 16 cores, 32 threads |
| **Memory** | 256 GB DDR5 ECC | 4x 64 GB |
| **Storage** | 2x 3.8 TB NVMe | RAID-1 |
| **Management** | BMC IPMI 2.0 | Redfish API |
| **Redundancy** | 1+1 | Hot-swap |

### 2.3 Плата портов (Line Card)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Forwarding** | Intel Tofino 2 | На каждой плате |
| **Ports** | 32x 400G QSFP-DD | или 64x 100G QSFP28 |
| **Transceivers** | QSFP-DD / QSFP28 | LR4, SR4, DR |
| **Memory** | 16 GB DDR4 | Per card |

### 2.4 Вентиляция

| Параметр | Значение |
|----------|----------|
| **Airflow** | Front-to-back |
| **Fans** | 6x hot-swap fans |
| **Cooling** | Forced air |

---

## 3. Сетевые возможности

### 3.1 Протоколы маршрутизации

| Протокол | Поддержка | Примечание |
|----------|-----------|------------|
| BGP-4 | ✓ | Full mesh, eBGP, iBGP |
| BGP-6 | ✓ | SRv6, SR-TE |
| ISIS | ✓ | Level 1/2 |
| OSPF | ✓ | Type 1/2/3/4/5/7 |
| MPLS | ✓ | LDP, RSVP-TE, MPLS-TP |
| Segment Routing | ✓ | SR-MPLS, SRv6 |
| PIM | ✓ | PIM-SM, PIM-DM |
| VRRP | ✓ | HA |
| LACP | ✓ | Link aggregation |
| ECMP | ✓ | До 256 paths |

### 3.2 Управление

| Метод | Поддержка |
|-------|-----------|
| NETCONF/YANG | ✓ RFC 6241/7950 |
| gRPC | ✓ Telemetry, streaming |
| SNMP | ✓ v2c/v3 |
| CLI | ✓ NX-OS-like |
| REST API | ✓ HTTPS |
| P4Runtime | ✓ Programmable pipeline |
| OpenFlow | ✓ 1.5 |

### 3.3 QoS

| Параметр | Значение |
|----------|----------|
| **Classes** | 64+ |
| **Queues** | 8 per port |
| **Scheduling** | WRR, WFQ, Strict |
| **Policing** | Token bucket, dual rate |
| **Marking** | DSCP, EXP, VLAN PCP |

### 3.4 HA (Высокая доступность)

| Параметр | Значение |
|----------|----------|
| **Failover** | < 50 ms |
| **Redundancy** | 1+1 control, N+1 power |
| **NSF** | ✓ Non-Stop Forwarding |
| **GR** | ✓ Graceful Restart |
| **SSO** | ✓ Stateful Switchover |

---

## 4. Программная платформа

### 4.1 Операционная система

| Параметр | Значение |
|----------|----------|
| **Base OS** | Yocto Project (Linux) |
| **Kernel** | 6.6 LTS (custom) |
| **Userspace** | DPDK 23.11, SPDK 23.11 |
| **Routing** | FRRouting 9.0+ |
| **Telemetry** | gRPC streaming, Prometheus |
| **Management** | Redfish, NETCONF |

### 4.2 DPDK-приложения

| Приложение | Назначение |
|------------|------------|
| `mts-forwarder` | High-speed packet forwarding |
| `mts-bgp` | BGP control plane |
| `mts-telemetry` | gRPC telemetry streaming |
| `mts-ctrl` | Management plane |

### 4.3 YANG-модели

```
mts-core-router/
├── mts-routing.yang
├── mts-interface.yang
├── mts-bgp.yang
├── mts-mpls.yang
├── mts-telemetry.yang
└── mts-ha.yang
```

---

## 5. Требования к Linux-драйверам

### 5.1 Intel Tofino 2

| Драйвер | Назначение |
|---------|------------|
| `tofino-p4` | P4 pipeline configuration |
| `tofino-phy` | PHY management |
| `tofino-ctrl` | Control plane interface |
| `tofino-telemetry` | Performance counters |

### 5.2 Broadcom TomTom

| Драйвер | Назначение |
|---------|------------|
| `tomtom-asic` | ASIC configuration |
| `tomtom-phy` | PHY management |

### 5.3 AMD EPYC

| Драйвер | Назначение |
|---------|------------|
| `amdfw` | Firmware management |
| `amdpcie` | PCIe tuning |

---

## 6. Энергопотребление

| Режим | Потребление |
|-------|-------------|
| **Idle** | 400W |
| **Load** | 1100W |
| **Peak** | 1200W |
| **Efficiency** | > 94% (80 PLUS Titanium) |

---

## 7. Физические параметры

| Параметр | Значение |
|----------|----------|
| **Размеры** | 442 x 445 x 44.5 mm (1RU) |
| **Вес** | 12 kg (max) |
| **Рабочая темп.** | 0°C ... +50°C |
| **Влажность** | 5% ... 95% non-condensing |
| **Шум** | < 50 dB(A) |

---

## 8. Трассировка платы

### 8.1 Основные сигналы

| Сигнал | Тип | Скорость | Примечание |
|--------|-----|----------|------------|
| PCIe Gen4 x16 | CPU ↔ Tofino 2 | 16 GT/s | 4 lanes per ASIC |
| DDR5 | CPU ↔ DDR5 | 4800 MT/s | ECC |
| I2C | BMC ↔ All | 100 kHz | Management |
| SPI | BMC ↔ Flash | 50 MHz | Boot |
| SFP MDIO | PHY ↔ ASIC | 100 kHz | PHY management |
| 1588v2 | All ports | SyncE | Clock distribution |

### 8.2 PCB параметры

| Параметр | Значение |
|----------|----------|
| **Layers** | 16-layer |
| **Material** | Rogers RO4350B (high freq) |
| **Impedance** | 50Ω single, 100Ω differential |
| **Copper** | 1 oz (outer), 0.5 oz (inner) |
| **Surface** | ENIG (gold finish) |

---

## 9. API спецификация (REST)

### 9.1 Основные эндпоинты

```
GET    /api/v1/routes          — Список маршрутов
POST   /api/v1/routes          — Добавить маршрут
DELETE /api/v1/routes/<id>     — Удалить маршрут
GET    /api/v1/interfaces      — Список интерфейсов
PUT    /api/v1/interfaces/<id> — Изменить интерфейс
GET    /api/v1/bgp             — BGP состояние
POST   /api/v1/bgp/neighbors   — Добавить neighbor
GET    /api/v1/telemetry       — Telemetry данные
GET    /api/v1/health          — Health check
```

### 9.2 Пример запроса

```json
{
  "method": "POST",
  "url": "/api/v1/routes",
  "body": {
    "prefix": "10.0.0.0/8",
    "next_hop": "192.168.1.1",
    "interface": "eth0",
    "protocol": "static",
    "priority": 10
  }
}
```

---

## 10. Checklist для разработки

- [ ] Проектирование main board (Tofino 2 + EPYC)
- [ ] Проектирование line card (Tofino 2 + ports)
- [ ] Проектирование control board (EPYC)
- [ ] Разработка драйверов Tofino 2
- [ ] Сборка Yocto образа
- [ ] Интеграция DPDK
- [ ] Настройка FRRouting
- [ ] Разработка YANG моделей
- [ ] Разработка REST API
- [ ] Тестирование HA
- [ ] Тестирование протоколов (BGP, MPLS, SRv6)
- [ ] Сертификация
