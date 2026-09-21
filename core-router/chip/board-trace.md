# MTS-CR-9000 — Трассировка платы (Main Board)

## 1. Архитектура платы

```
┌─────────────────────────────────────────────────────────────────────┐
│                        MTS-CR-9000 MAIN BOARD                      │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐    │
│  │ Intel    │    │ Intel    │    │ Intel    │    │ Intel    │    │
│  │ Tofino 2 │    │ Tofino 2 │    │ Tofino 2 │    │ Tofino 2 │    │
│  │ (ASIC)   │    │ (ASIC)   │    │ (ASIC)   │    │ (ASIC)   │    │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘    │
│       │               │               │               │            │
│  ┌────▼───────────────▼───────────────▼───────────────▼─────┐     │
│  │              AMD EPYC 7003 (Rome) CPU                     │     │
│  │              32 cores / 64 threads @ 2.5 GHz              │     │
│  └────┬───────────────┬───────────────┬───────────────┬─────┘     │
│       │               │               │               │            │
│  ┌────▼─────┐  ┌─────▼────┐  ┌─────▼────┐  ┌─────▼────┐        │
│  │ DDR5     │  │ DDR5     │  │ DDR5     │  │ DDR5     │        │
│  │ 128 GB   │  │ 128 GB   │  │ 128 GB   │  │ 128 GB   │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ NVMe     │  │ NVMe     │  │ NVMe     │  │ NVMe     │        │
│  │ 3.8 TB   │  │ 3.8 TB   │  │ 3.8 TB   │  │ 3.8 TB   │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ BMC      │  │ Power    │  │ Fan      │  │ Management│       │
│  │ IPMI     │  │ Mgmt     │  │ Ctrl     │  │ IC       │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. PCIe трассировка

### 2.1 CPU ↔ Tofino 2 (4x PCIe Gen4 x16)

| Lane | CPU Pin | Tofino 2 Pin | Impedance | Length |
|------|---------|--------------|-----------|--------|
| PCIe 0 Lane 0 | J12A1 | K23 | 50Ω | < 15 mm |
| PCIe 0 Lane 1 | J12A2 | K24 | 50Ω | < 15 mm |
| PCIe 0 Lane 2 | J12A3 | K25 | 50Ω | < 15 mm |
| PCIe 0 Lane 3 | J12A4 | K26 | 50Ω | < 15 mm |
| PCIe 0 Lane 4 | J12A5 | K27 | 50Ω | < 15 mm |
| PCIe 0 Lane 5 | J12A6 | K28 | 50Ω | < 15 mm |
| PCIe 0 Lane 6 | J12A7 | K29 | 50Ω | < 15 mm |
| PCIe 0 Lane 7 | J12A8 | K30 | 50Ω | < 15 mm |
| PCIe 0 Lane 8 | J12B1 | K31 | 50Ω | < 15 mm |
| PCIe 0 Lane 9 | J12B2 | K32 | 50Ω | < 15 mm |
| PCIe 0 Lane 10 | J12B3 | K33 | 50Ω | < 15 mm |
| PCIe 0 Lane 11 | J12B4 | K34 | 50Ω | < 15 mm |
| PCIe 0 Lane 12 | J12B5 | K35 | 50Ω | < 15 mm |
| PCIe 0 Lane 13 | J12B6 | K36 | 50Ω | < 15 mm |
| PCIe 0 Lane 14 | J12B7 | K37 | 50Ω | < 15 mm |
| PCIe 0 Lane 15 | J12B8 | K38 | 50Ω | < 15 mm |

### 2.2 CPU ↔ BMC (PCIe Gen3 x4)

| Lane | CPU Pin | BMC Pin | Impedance | Length |
|------|---------|---------|-----------|--------|
| PCIe 1 Lane 0 | L15A1 | M10 | 50Ω | < 10 mm |
| PCIe 1 Lane 1 | L15A2 | M11 | 50Ω | < 10 mm |
| PCIe 1 Lane 2 | L15A3 | M12 | 50Ω | < 10 mm |
| PCIe 1 Lane 3 | L15A4 | M13 | 50Ω | < 10 mm |

### 2.3 CPU ↔ NVMe (4x PCIe Gen4 x4)

| Lane | CPU Pin | NVMe Pin | Impedance | Length |
|------|---------|----------|-----------|--------|
| PCIe 2 Lane 0 | N18A1 | P20 | 50Ω | < 15 mm |
| PCIe 3 Lane 0 | N18B1 | P21 | 50Ω | < 15 mm |
| PCIe 4 Lane 0 | N18C1 | P22 | 50Ω | < 15 mm |
| PCIe 5 Lane 0 | N18D1 | P23 | 50Ω | < 15 mm |

## 3. DDR5 трассировка

### 3.1 Memory channels

| Channel | DQ Pins | Address Pins | CLK | Length |
|---------|---------|-------------|-----|--------|
| CH0 | A1-A8 | B1-B4 | C1 | < 5 mm |
| CH1 | A9-A16 | B5-B8 | C2 | < 5 mm |
| CH2 | A17-A24 | B9-B12 | C3 | < 5 mm |
| CH3 | A25-A32 | B13-B16 | C4 | < 5 mm |

### 3.2 DDR5 timing

| Parameter | Value |
|-----------|-------|
| **Frequency** | 4800 MT/s |
| **VDD** | 1.1 V |
| **VDDQ** | 1.1 V |
| **Impedance** | 40Ω differential |
| **Length match** | < 0.5 mm |

## 4. I2C трассировка

### 4.1 BMC ↔ All devices

| Signal | BMC Pin | Target | Pull-up |
|--------|---------|--------|---------|
| SDA0 | D20 | Tofino 2 | 4.7kΩ |
| SCL0 | D21 | Tofino 2 | 4.7kΩ |
| SDA1 | D22 | BMC | 4.7kΩ |
| SCL1 | D23 | BMC | 4.7kΩ |
| SDA2 | D24 | Power | 4.7kΩ |
| SCL2 | D25 | Power | 4.7kΩ |
| SDA3 | D26 | Fan | 4.7kΩ |
| SCL3 | D27 | Fan | 4.7kΩ |

## 5. SPI трассировка

### 5.1 BMC ↔ Flash

| Signal | BMC Pin | Flash Pin | Length |
|--------|---------|-----------|--------|
| MOSI | E30 | PIN 5 | < 5 mm |
| MISO | E31 | PIN 2 | < 5 mm |
| SCK | E32 | PIN 6 | < 5 mm |
| CS | E33 | PIN 4 | < 5 mm |

## 6. PCB параметры

| Параметр | Значение |
|----------|----------|
| **Layers** | 16-layer |
| **Material** | Rogers RO4350B (high freq) |
| **Substrate** | FR-4 (low freq) |
| **Impedance** | 50Ω single, 100Ω differential |
| **Copper** | 1 oz (outer), 0.5 oz (inner) |
| **Surface** | ENIG (gold finish) |
| **Via** | Microvia (0.15 mm), Blind (0.3 mm) |
| **Thickness** | 3.2 mm |
