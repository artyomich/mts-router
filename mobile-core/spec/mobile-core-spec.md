# MTS-MC-5000 — Mobile Core Router

## Спецификация маршрутизатора ядра сотовой сети (EPC/5GC)

---

## 1. Назначение

MTS-MC-5000 — маршрутизатор для ядра сотовой сети МТС, обеспечивающий:
- User Plane Function (UPF) — пересылка пользовательского трафика
- Session Management Function (SMF) — управление сессиями
- Policy Control Function (PCF) — контроль политик
- PFCP (Packet Forwarding Control Protocol) — управление user plane
- GTP-U/GTP-C — протоколы сотовой связи
- 5G core network (5GC) — полное ядро 5G

---

## 2. Аппарная спецификация

### 2.1 Основная плата (Main Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Forwarding** | Marvell ThunderX3 | ARM64 Neoverse V1, 96 cores |
| **CPU** | AMD EPYC 7002 (Naples) | 32 cores, 64 threads |
| **Memory** | 256 GB DDR4 ECC | 8x 32 GB RDIMM |
| **Storage** | 2x 3.8 TB NVMe SSD | RAID-1, boot + logs |
| **Fabric** | PCIe 4.0 x16 | CPU ↔ ThunderX3 |
| **Power** | 800W AC/DC | Redundant 1+1 |
| **Dimensions** | 1RU | 442 x 445 x 44.5 mm |

### 2.2 Плата портов (Line Card)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Ports** | 32x 100G QSFP28 | N3/N6 interfaces |
| **Backup** | 64x 25G SFP28 | N2/N4 interfaces |
| **Transceivers** | LR4, SR4, DR | Multi-mode |
| **Memory** | 8 GB DDR4 | Per card |

### 2.3 Вентиляция

| Параметр | Значение |
|----------|----------|
| **Airflow** | Front-to-back |
| **Fans** | 6x hot-swap fans |
| **Cooling** | Forced air |

---

## 3. Сетевые возможности

### 3.1 Протоколы сотовой связи

| Протокол | Версия | Поддержка |
|----------|--------|-----------|
| GTP-U | 14.x | ✓ User plane |
| GTP-C | 14.x | ✓ Control plane |
| PFCP | 1.2.0 | ✓ 5GC |
| Diameter | 3.0 | ✓ EPC |
| HTTP/2 | 2.0 | ✓ 5GC (AMF/SMF) |
| SCTP | RFC 4960 | ✓ Diameter |
| UDP/TCP | — | ✓ GTP-U |

### 3.2 Протоколы маршрутизации

| Протокол | Поддержка |
|----------|-----------|
| BGP-4 | ✓ |
| OSPF | ✓ |
| ISIS | ✓ |
| LACP | ✓ |
| ECMP | ✓ |

### 3.3 QoS для сотовой сети

| Параметр | Значение |
|----------|----------|
| **QoS Classes** | 5QI (5G QoS Identifier) |
| **5QI types** | GBR, non-GBR, delay-critical |
| **Priorities** | 8 per UE |
| **Packet Filtering** | PCC rules |
| **UL/DL Counters** | Per QoS flow |

### 3.4 HA

| Параметр | Значение |
|----------|----------|
| **Failover** | < 50 ms |
| **Redundancy** | 1+1 control |
| **NSF** | ✓ |
| **GR** | ✓ |

---

## 4. Программная платформа

### 4.1 Операционная система

| Параметр | Значение |
|----------|----------|
| **Base OS** | Yocto Project (Linux) |
| **Kernel** | 6.6 LTS (custom) |
| **Orchestration** | K3s (Kubernetes) |
| **Userspace** | DPDK 23.11 |
| **Control Plane** | 5GC NFs (UPF, SMF, AMF) |
| **Telemetry** | Prometheus, Grafana |

### 4.2 Контейнеризованные NF (Network Functions)

| Функция | Образ | Назначение |
|---------|-------|------------|
| UPF | `mts/upf:latest` | User plane forwarding |
| SMF | `mts/smf:latest` | Session management |
| AMF | `mts/amf:latest` | Access management |
| PCF | `mts/pcf:latest` | Policy control |
| UDM | `mts/udm:latest` | User data management |
| NSSF | `mts/nssf:latest` | Network selection |
| AUSF | `mts/ausf:latest` | Authentication |
| NRF | `mts/nrf:latest` | Network repository |

### 4.3 YANG-модели

```
mts-mobile-core/
├── mts-5gc-upf.yang
├── mts-5gc-smf.yang
├── mts-gtp.yang
├── mts-pfcp.yang
├── mts-qos.yang
└── mts-telemetry.yang
```

---

## 5. Требования к Linux-драйверам

### 5.1 Marvell ThunderX3

| Драйвер | Назначение |
|---------|------------|
| `thunderx3-net` | Ethernet TX/RX |
| `thunderx3-pmu` | Performance counters |
| `thunderx3-cxl` | CXL memory management |

### 5.2 AMD EPYC

| Драйвер | Назначение |
|---------|------------|
| `amdfw` | Firmware management |
| `amdpcie` | PCIe tuning |

### 5.3 DPDK PMD (Poll Mode Driver)

| PMD | Назначение |
|-----|------------|
| `dpdk-mvneta` | Marvell network |
| `dpdk-netvsc` | Virtual network |
| `dpdk-ring` | Internal forwarding |

---

## 6. Энергопотребление

| Режим | Потребление |
|-------|-------------|
| **Idle** | 300W |
| **Load** | 700W |
| **Peak** | 800W |

---

## 7. Физические параметры

| Параметр | Значение |
|----------|----------|
| **Размеры** | 442 x 445 x 44.5 mm (1RU) |
| **Вес** | 10 kg (max) |
| **Рабочая темп.** | 0°C ... +50°C |
| **Влажность** | 5% ... 95% non-condensing |
| **Шум** | < 50 dB(A) |

---

## 8. API спецификация (REST)

### 8.1 Основные эндпоинты

```
GET    /api/v1/upf                 — UPF состояние
POST   /api/v1/upf/sessions        — Создать session
DELETE /api/v1/upf/sessions/<id>   — Удалить session
GET    /api/v1/pfcp                — PFCP состояние
POST   /api/v1/pfcp/steering       — Steering rule
GET    /api/v1/gtp                 — GTP состояние
GET    /api/v1/5qis               — 5QI конфигурация
GET    /api/v1/nrf                — NRF registry
GET    /api/v1/health             — Health check
```

### 8.2 Пример запроса (создание PDU session)

```json
{
  "method": "POST",
  "url": "/api/v1/upf/sessions",
  "body": {
    "ue_ip": "10.64.0.1",
    "pnni": "ims.mts.ru",
    "qfi": 1,
    "5qi": 9,
    "upf_ip": "192.168.10.1",
    "teid": 0x12345
  }
}
```

---

## 9. Трассировка платы

### 9.1 Основные сигналы

| Сигнал | Тип | Скорость | Примечание |
|--------|-----|----------|------------|
| PCIe Gen4 x16 | CPU ↔ ThunderX3 | 16 GT/s | |
| DDR4 | CPU ↔ DDR4 | 3200 MT/s | ECC |
| I2C | BMC ↔ All | 100 kHz | Management |
| SPI | BMC ↔ Flash | 50 MHz | Boot |
| 1588v2 | All ports | SyncE | Clock |

### 9.2 PCB параметры

| Параметр | Значение |
|----------|----------|
| **Layers** | 16-layer |
| **Material** | Rogers RO4350B |
| **Impedance** | 50Ω single, 100Ω differential |
| **Copper** | 1 oz (outer), 0.5 oz (inner) |
| **Surface** | ENIG |

---

## 10. Checklist для разработки

- [ ] Проектирование main board (ThunderX3 + EPYC)
- [ ] Проектирование line card (100G ports)
- [ ] Разработка драйверов ThunderX3
- [ ] Сборка Yocto образа + K3s
- [ ] Интеграция DPDK
- [ ] Разработка UPF (libteut)
- [ ] Разработка SMF/AMF/PCF
- [ ] Разработка YANG моделей
- [ ] Разработка REST API
- [ ] Тестирование GTP-U
- [ ] Тестирование PFCP
- [ ] Тестирование HA
- [ ] Сертификация 3GPP
