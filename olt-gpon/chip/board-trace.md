# MTS-OLT-2000 — Трассировка платы (Main Board)

## 1. Архитектура платы

```
┌─────────────────────────────────────────────────────────────────────┐
│                       MTS-OLT-2000 MAIN BOARD                       │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐    │
│  │ Intel    │    │ AMD      │    │ Intel    │    │ Intel    │    │
│  │ Tofino 2 │    │ EPYC     │    │ Tofino 2 │    │ Tofino 2 │    │
│  │ (FWD)    │    │ 3000     │    │ (CTRL)   │    │ (MGMT)   │    │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘    │
│       │               │               │               │            │
│  ┌────▼───────────────▼───────────────▼───────────────▼─────┐     │
│  │              Realtek RTL960x GPON Line Interface         │     │
│  │              192x ONU / OMCI / WDM Management            │     │
│  └────┬─────────────────────────────────────────────────────┘     │
│       │                                                            │
│  ┌────▼─────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ DDR4     │  │ EEPROM   │  │ Fan      │  │ Power    │        │
│  │ 128 GB   │  │ 24C64    │  │ Ctrl     │  │ Mgmt     │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. PCIe трассировка

### 2.1 CPU ↔ Tofino 2 (4x PCIe Gen4 x16)

| Lane | CPU Pin | Tofino Pin | Impedance | Length |
|------|---------|------------|-----------|--------|
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

### 2.2 CPU ↔ RTL960x (PCIe Gen3 x4)

| Lane | CPU Pin | RTL960x Pin | Impedance | Length |
|------|---------|-------------|-----------|--------|
| PCIe 1 Lane 0 | L15A1 | M20 | 85Ω | < 20 mm |
| PCIe 1 Lane 1 | L15A2 | M21 | 85Ω | < 20 mm |
| PCIe 1 Lane 2 | L15A3 | M22 | 85Ω | < 20 mm |
| PCIe 1 Lane 3 | L15A4 | M23 | 85Ω | < 20 mm |

### 2.3 CPU ↔ BMC (PCIe Gen3 x4)

| Lane | CPU Pin | BMC Pin | Impedance | Length |
|------|---------|---------|-----------|--------|
| PCIe 2 Lane 0 | N18A1 | P10 | 50Ω | < 10 mm |
| PCIe 2 Lane 1 | N18A2 | P11 | 50Ω | < 10 mm |
| PCIe 2 Lane 2 | N18A3 | P12 | 50Ω | < 10 mm |
| PCIe 2 Lane 3 | N18A4 | P13 | 50Ω | < 10 mm |

## 3. DDR4 трассировка

### 3.1 Memory channels

| Channel | DQ Pins | Address Pins | CLK | Length |
|---------|---------|-------------|-----|--------|
| CH0 | A1-A8 | B1-B4 | C1 | < 5 mm |
| CH1 | A9-A16 | B5-B8 | C2 | < 5 mm |
| CH2 | A17-A24 | B9-B12 | C3 | < 5 mm |
| CH3 | A25-A32 | B13-B16 | C4 | < 5 mm |

### 3.2 DDR4 timing

| Parameter | Value |
|-----------|-------|
| **Frequency** | 2400 MT/s |
| **VDD** | 1.2 V |
| **VDDQ** | 1.2 V |
| **Impedance** | 40Ω single-ended |
| **Length match** | < 0.5 mm |

## 4. GPON трассировка

### 4.1 RTL960x ↔ WDM multiplexer

| Signal | RTL960x Pin | WDM Pin | Type | Length |
|--------|-------------|---------|------|--------|
| TX1 | N14A1 | W1-TX1 | LVDS | < 30 mm |
| TX2 | N14A2 | W1-TX2 | LVDS | < 30 mm |
| RX1 | N15A1 | W1-RX1 | LVDS | < 30 mm |
| RX2 | N15A2 | W1-RX2 | LVDS | < 30 mm |

### 4.2 RTL960x ↔ ONU connectors (192 ports)

| Group | Pins | ONU Range | Connector Type |
|-------|------|-----------|----------------|
| Group 1 | O1-O48 | ONU 1-48 | SC/APC x4 |
| Group 2 | O49-O96 | ONU 49-96 | SC/APC x4 |
| Group 3 | O97-O144 | ONU 97-144 | SC/APC x4 |
| Group 4 | O145-O192 | ONU 145-192 | SC/APC x4 |

## 5. 10G uplink трассировка

### 5.1 Tofino 2 ↔ QSFP+ connectors

| Port | Tofino Pin | Connector | Type | Length |
|------|------------|-----------|------|--------|
| Uplink 0 | K20A1 | Z1 | QSFP+ 10G | < 100 mm |
| Uplink 1 | K20A2 | Z2 | QSFP+ 10G | < 100 mm |
| Uplink 2 | K20A3 | Z3 | QSFP+ 10G | < 100 mm |
| Uplink 3 | K20A4 | Z4 | QSFP+ 10G | < 100 mm |

## 6. I2C трассировка

### 6.1 CPU ↔ All devices

| Signal | CPU Pin | Target | Pull-up | Resistor |
|--------|---------|--------|---------|----------|
| I2C_SDA | N16A1 | RTL960x/EEPROM/PSU | 4.7kΩ | R1-R3 |
| I2C_SCL | N16A2 | RTL960x/EEPROM/PSU | 4.7kΩ | R1-R3 |
| SMB_ALERT | N16A3 | PSU/Fan | 4.7kΩ | R4 |

## 7. Clock трассировка

### 7.1 Reference clocks

| Clock | Source | Fan-out | Amplitude | Length |
|-------|--------|---------|-----------|--------|
| 125 MHz REF | Tofino 2 | 4 outputs | 800mVpp | < 50 mm |
| 25 MHz REF | Tofino 2 | 2 outputs | 800mVpp | < 50 mm |
| 1588 PTP | Tofino 2 | 2 outputs | 1.8V | < 50 mm |

## 8. Power delivery

### 8.1 Power rails

| Rail | Voltage | Current | Regulator | Location |
|------|---------|---------|-----------|----------|
| VCCINT | 0.8V | 80A | TI TPS546D2A | Tofino 2 |
| VCCDDRO | 0.8V | 20A | TI TPS546D2A | DDR4 |
| VCC AUX | 3.3V | 10A | ON Semi NCP3031 | Edge |
| VBAT | 3.3V | 100mA | MCP1825 | BMC area |

### 8.2 Power sequencing

| Step | Rail | Voltage | Ramp Time | Enable |
|------|------|---------|-----------|--------|
| 1 | VCC AUX | 3.3V | 1ms | PWR_GOOD |
| 2 | VBAT | 3.3V | 0.5ms | PWR_GOOD |
| 3 | VCCINT | 0.8V | 2ms | VCC AUX OK |
| 4 | VCCDDRO | 0.8V | 2ms | VCCINT OK |
| 5 | PCIe PERST# | — | — | All rails OK |
