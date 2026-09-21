# MTS-OLT-2000 — GPON OLT Equipment

## Спецификация OLT-оборудования для GPON-доступа

---

## 1. Назначение

MTS-OLT-2000 — OLT-оборудование для GPON-доступа МТС, обеспечивающее:
- GPON/EPON мультиплексирование для FTTH
- Подключение до 4096 ONU на устройство
- OMCI/ONT management protocol
- TR-069 для remote management
- High-speed uplink to core network

---

## 2. Аппарная спецификация

### 2.1 Основная плата (Main Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Forwarding** | Intel Tofino 2 | P4-programmable |
| **CPU** | AMD EPYC 7002 | 32 cores, 64 threads |
| **Memory** | 32 GB DDR4 ECC | |
| **Storage** | 2x 480 GB SSD | RAID-1 |
| **Management** | BMC IPMI 2.0 | Redfish API |
| **Power** | 500W AC/DC | Redundant 1+1 |
| **Dimensions** | 1RU | 442 x 445 x 44.5 mm |

### 2.2 GPON Line Card

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **GPON PHY** | Realtek RTL960x | GPON PHY + MAC |
| **Ports** | 16x GPON ports | 2.5 Gbps down, 1.25 Gbps up |
| **ONU** | до 128 ONU per port | TDM split 1:128 |
| **Memory** | 2 GB DDR4 | Per card |
| **WDM** | ITU-T G.984 | G.984.1/G.984.4 |

### 2.3 Uplink Card

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **Ports** | 4x 10G SFP+ | Uplink to core |
| **Ports** | 2x 100G QSFP28 | High-capacity uplink |
| **Redundancy** | 1+1 | Hot-swap |

### 2.4 Вентиляция

| Параметр | Значение |
|----------|----------|
| **Airflow** | Front-to-back |
| **Fans** | 4x hot-swap fans |
| **Cooling** | Forced air |

---

## 3. Сетевые возможности

### 3.1 GPON спецификация

| Параметр | Значение |
|----------|----------|
| **Standard** | ITU-T G.984 (GPON) |
| **Downstream** | 2.488 Gbps |
| **Upstream** | 1.244 Gbps |
| **Split Ratio** | 1:128 (standard), 1:256 (extended) |
| **Distance** | до 60 km |
| **ONU per port** | до 128 |
| **ONU total** | до 2048 (16 ports) |

### 3.2 Протоколы

| Протокол | Версия | Поддержка |
|----------|--------|-----------|
| GPON | G.984 | ✓ |
| EPON | IEEE 802.3ah | ✓ |
| OMCI | ME-198 | ✓ |
| TR-069 | CWMP | ✓ |
| DHCP | RFC 2131 | ✓ |
| PPPoE | RFC 2516 | ✓ |
| VLAN | 802.1Q | ✓ |
| QoS | 802.1p | ✓ |
| IGMP | v2/v3 | ✓ |
| Multicast | IGMP Snooping | ✓ |

### 3.3 QoS для GPON

| Параметр | Значение |
|----------|----------|
| **Classes** | 8 per ONU |
| **Scheduling** | WRR, Strict |
| **Policing** | Dual-rate, triple-rate |
| **Marking** | DSCP, EXP, VLAN PCP |

### 3.4 HA

| Параметр | Значение |
|----------|----------|
| **Failover** | < 50 ms |
| **Redundancy** | 1+1 power, N+1 line card |
| **NSF** | ✓ |
| **GR** | ✓ |

---

## 4. Программная платформа

### 4.1 Операционная система

| Параметр | Значение |
|----------|----------|
| **Base OS** | OpenWrt (Linux) |
| **Kernel** | 6.6 LTS (custom) |
| **GPON drivers** | Custom RTL960x drivers |
| **Management** | TR-069, OMCI, NETCONF |
| **Web UI** | LuCI |
| **CLI** | OpenWrt CLI |

### 4.2 YANG-модели

```
mts-olt-gpon/
├── mts-olt-gpon.yang
├── mts-olt-onu.yang
├── mts-olt-tr069.yang
├── mts-olt-omci.yang
├── mts-olt-qos.yang
└── mts-olt-telemetry.yang
```

---

## 5. Требования к Linux-драйверам

### 5.1 Intel Tofino 2

| Драйвер | Назначение |
|---------|------------|
| `tofino-p4` | P4 pipeline configuration |
| `tofino-phy` | PHY management |
| `tofino-ctrl` | Control plane interface |

### 5.2 Realtek RTL960x

| Драйвер | Назначение |
|---------|------------|
| `rtl960x-gpon` | GPON PHY + MAC |
| `rtl960x-omci` | OMCI management |
| `rtl960x-wdm` | WDM filtering |

### 5.3 AMD EPYC

| Драйвер | Назначение |
|---------|------------|
| `amdfw` | Firmware management |
| `amdpcie` | PCIe tuning |

---

## 6. Энергопотребление

| Режим | Потребление |
|-------|-------------|
| **Idle** | 200W |
| **Load** | 450W |
| **Peak** | 500W |

---

## 7. Физические параметры

| Параметр | Значение |
|----------|----------|
| **Размеры** | 442 x 445 x 44.5 mm (1RU) |
| **Вес** | 8 kg (max) |
| **Рабочая темп.** | 0°C ... +50°C |
| **Влажность** | 5% ... 95% non-condensing |
| **Шум** | < 45 dB(A) |
| **Protection** | IP30 |

---

## 8. API спецификация (REST)

### 8.1 Основные эндпоинты

```
GET    /api/v1/olt                — OLT state
GET    /api/v1/onu                — ONU list
GET    /api/v1/onu/<id>          — ONU details
PUT    /api/v1/onu/<id>          — ONU config
POST   /api/v1/onu/<id>/reset    — ONU reset
GET    /api/v1/tr069             — TR-069 config
GET    /api/v1/gpon              — GPON state
GET    /api/v1/telemetry         — Telemetry
GET    /api/v1/health            — Health check
```

### 8.2 Пример запроса (конфигурация ONU)

```json
{
  "method": "PUT",
  "url": "/api/v1/onu/001",
  "body": {
    "pon_port": 1,
    "onu_id": 10,
    "vlan": 100,
    "qos_profile": "gold",
    "bandwidth_up": "100 Mbps",
    "bandwidth_down": "300 Mbps"
  }
}
```

---

## 9. Трассировка платы

### 9.1 Основные сигналы

| Сигнал | Тип | Скорость | Примечание |
|--------|-----|----------|------------|
| PCIe Gen4 x16 | CPU ↔ Tofino 2 | 16 GT/s | |
| PCIe Gen3 x4 | CPU ↔ GPON card | 8 GT/s | |
| DDR4 | CPU ↔ DDR4 | 3200 MT/s | ECC |
| I2C | BMC ↔ All | 100 kHz | Management |
| SPI | BMC ↔ Flash | 50 MHz | Boot |
| GPON | GPON card → ONU | 2.5 Gbps | Downstream |
| GPON | ONU → GPON card | 1.25 Gbps | Upstream |

### 9.2 PCB параметры

| Параметр | Значение |
|----------|----------|
| **Layers** | 12-layer |
| **Material** | Rogers RO4350B (high freq) |
| **Impedance** | 50Ω single, 100Ω differential |
| **Copper** | 1 oz (outer), 0.5 oz (inner) |
| **Surface** | ENIG |

---

## 10. Checklist для разработки

- [ ] Проектирование main board (Tofino 2 + EPYC)
- [ ] Проектирование GPON line card (RTL960x)
- [ ] Проектирование uplink card (10G/100G)
- [ ] Разработка драйверов Tofino 2
- [ ] Разработка драйверов RTL960x
- [ ] Сборка OpenWrt образа
- [ ] Интеграция TR-069
- [ ] Разработка OMCI management
- [ ] Разработка YANG моделей
- [ ] Разработка REST API
- [ ] Тестирование GPON
- [ ] Тестирование ONU management
- [ ] Тестирование HA
- [ ] Сертификация ITU-T G.984
