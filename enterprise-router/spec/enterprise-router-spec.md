# MTS-ER-1000 — Enterprise Router

## Спецификация корпоративного маршрутизатора (B2B)

---

## 1. Назначение

MTS-ER-1000 — корпоративный маршрутизатор для B2B-сегмента МТС, обеспечивающий:
- Выделенные каналы для бизнеса (MPLS, IPsec)
- SD-WAN для множественных WAN
- Подключение к cloud (AWS, Azure, GCP)
- High availability для критичных приложений
- QoS для VoIP, Video, Data

---

## 2. Аппарная спецификация

### 2.1 Основная плата (Main Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **SoC** | NXP S32G3 | ARM Cortex-A53/A72, 8 cores |
| **Switch** | Broadcom TomTom | 10G/1G ports |
| **Memory** | 8 GB DDR4 ECC | |
| **Storage** | 128 GB eMMC | Boot + OS |
| **Crypto** | NXP SEC (Hardware) | IPsec, TLS |
| **Power** | 100W AC/DC | Redundant 1+1 |
| **Dimensions** | 1RU | 442 x 445 x 44.5 mm |

### 2.2 Плата портов (Line Card)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Ports** | 4x 10G SFP+ | WAN/Upstream |
| **Ports** | 8x 1G SFP | LAN/Downstream |
| **WAN** | DSL, Fiber, Copper | Multi-WAN |
| **Memory** | 512 MB DDR4 | Per card |

### 2.3 Вентиляция

| Параметр | Значение |
|----------|----------|
| **Airflow** | Front-to-back |
| **Fans** | 2x hot-swap fans |
| **Cooling** | Passive + forced |

---

## 3. Сетевые возможности

### 3.1 Протоколы

| Протокол | Версия | Поддержка |
|----------|--------|-----------|
| MPLS | LDP, RSVP-TE | ✓ |
| BGP-4 | — | ✓ |
| OSPF | Type 1/2/3/5 | ✓ |
| ISIS | Level 1/2 | ✓ |
| IPsec | RFC 4301 | ✓ |
| L2TP | RFC 3931 | ✓ |
| SD-WAN | — | ✓ |
| BFD | RFC 3718 | ✓ |
| LACP | — | ✓ |
| VRRP | RFC 5798 | ✓ |

### 3.2 SD-WAN

| Функция | Поддержка |
|---------|-----------|
| **WAN types** | MPLS, Internet, LTE, 5G |
| **Load balancing** | Per-flow, per-application |
| **Failover** | < 50 ms |
| **Path selection** | Application-aware |
| **Encryption** | IPsec, TLS |
| **Orchestration** | Centralized controller |

### 3.3 QoS

| Параметр | Значение |
|----------|----------|
| **Classes** | 8 per port |
| **Scheduling** | WRR, WFQ, Strict |
| **Policing** | Token bucket |
| **Marking** | DSCP, EXP |

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
| **Base OS** | OpenWrt (Linux) |
| **Kernel** | 6.6 LTS (custom) |
| **Routing** | FRRouting 9.0+ |
| **SD-WAN** | Custom engine |
| **IPsec** | strongSwan |
| **Management** | NETCONF, CLI, Web UI |

### 4.2 YANG-модели

```
mts-enterprise-router/
├── mts-er-sdwan.yang
├── mts-er-mpls.yang
├── mts-er-ipsec.yang
├── mts-er-qos.yang
└── mts-er-telemetry.yang
```

---

## 5. Требования к Linux-драйверам

### 5.1 NXP S32G3

| Драйвер | Назначение |
|---------|------------|
| `s32g-net` | Ethernet TX/RX |
| `s32g-sec` | Hardware crypto (IPsec) |

### 5.2 Broadcom TomTom

| Драйвер | Назначение |
|---------|------------|
| `tomtom-asic` | ASIC configuration |
| `tomtom-phy` | PHY management |

---

## 6. Энергопотребление

| Режим | Потребление |
|-------|-------------|
| **Idle** | 40W |
| **Load** | 90W |
| **Peak** | 100W |

---

## 7. Физические параметры

| Параметр | Значение |
|----------|----------|
| **Размеры** | 442 x 445 x 44.5 mm (1RU) |
| **Вес** | 3 kg (max) |
| **Рабочая темп.** | 0°C ... +50°C |
| **Влажность** | 5% ... 95% non-condensing |
| **Шум** | < 35 dB(A) |
| **Protection** | IP30 |

---

## 8. API спецификация (REST)

### 8.1 Основные эндпоинты

```
GET    /api/v1/sdwan              — SD-WAN state
POST   /api/v1/sdwan/path         — Add path
PUT    /api/v1/sdwan/path/<id>    — Update path
GET    /api/v1/mpls               — MPLS state
POST   /api/v1/mpls/lsp           — Create LSP
GET    /api/v1/ipsec              — IPsec state
POST   /api/v1/ipsec/tunnel       — Create tunnel
GET    /api/v1/interfaces         — Interface list
GET    /api/v1/telemetry         — Telemetry
GET    /api/v1/health            — Health check
```

### 8.2 Пример запроса (SD-WAN path)

```json
{
  "method": "POST",
  "url": "/api/v1/sdwan/path",
  "body": {
    "wan_interface": "eth1",
    "type": "mpls",
    "priority": "high",
    "qos_profile": "gold",
    "failover_interface": "eth2"
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

- [ ] Проектирование main board (S32G3 + TomTom)
- [ ] Проектирование line card (10G/1G ports)
- [ ] Разработка драйверов S32G3
- [ ] Сборка OpenWrt образа
- [ ] Разработка SD-WAN engine
- [ ] Интеграция FRRouting
- [ ] Разработка YANG моделей
- [ ] Разработка REST API
- [ ] Тестирование MPLS
- [ ] Тестирование IPsec
- [ ] Тестирование HA
- [ ] Сертификация
