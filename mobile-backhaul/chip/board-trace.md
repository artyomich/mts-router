# MTS-MB-3000 — Трассировка платы (Main Board)

## 1. Архитектура платы

```
┌─────────────────────────────────────────────────────────────────────┐
│                      MTS-MB-3000 MAIN BOARD                        │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐    │
│  │ NXP      │    │ NXP      │    │ NXP      │    │ NXP      │    │
│  │ S32G3    │    │ S32G3    │    │ S32G3    │    │ S32G3    │    │
│  │ (CORE)   │    │ (CORE)   │    │ (CORE)   │    │ (CORE)   │    │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘    │
│       │               │               │               │            │
│  ┌────▼───────────────▼───────────────▼───────────────▼─────┐     │
│  │              Marvell 88Q5242 10-Port Switch               │     │
│  │              8x 10G SFP+ / 2x 100G QSFP+                  │     │
│  └────┬─────────────────────────────────────────────────────┘     │
│       │                                                            │
│  ┌────▼─────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ DDR4     │  │ EEPROM   │  │ Fan      │  │ Power    │        │
│  │ 256 MB   │  │ 24C64    │  │ Ctrl     │  │ Mgmt     │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. PCIe трассировка

### 2.1 S32G3 ↔ 88Q5242 Switch (PCIe Gen3 x4)

| Lane | S32G3 Pin | Switch Pin | Impedance | Length |
|------|-----------|------------|-----------|--------|
| PCIe 0 Lane 0 | K12A1 | L23 | 85Ω | < 15 mm |
| PCIe 0 Lane 1 | K12A2 | L24 | 85Ω | < 15 mm |
| PCIe 0 Lane 2 | K12A3 | L25 | 85Ω | < 15 mm |
| PCIe 0 Lane 3 | K12A4 | L26 | 85Ω | < 15 mm |

### 2.2 S32G3 ↔ BMC (PCIe Gen2 x2)

| Lane | S32G3 Pin | BMC Pin | Impedance | Length |
|------|-----------|---------|-----------|--------|
| PCIe 1 Lane 0 | K13A1 | M10 | 85Ω | < 10 mm |
| PCIe 1 Lane 1 | K13A2 | M11 | 85Ω | < 10 mm |

## 3. DDR4 трассировка

### 3.1 Memory channels

| Channel | DQ Pins | Address Pins | CLK | Length |
|---------|---------|-------------|-----|--------|
| CH0 | A1-A8 | B1-B4 | C1 | < 5 mm |
| CH1 | A9-A16 | B5-B8 | C2 | < 5 mm |

### 3.2 DDR4 timing

| Parameter | Value |
|-----------|-------|
| **Frequency** | 1866 MT/s |
| **VDD** | 1.2 V |
| **VDDQ** | 1.2 V |
| **Impedance** | 40Ω single-ended |
| **Length match** | < 0.5 mm |

## 4. Ethernet port трассировка

### 4.1 88Q5242 ↔ SFP+ connectors (10G ports)

| Port | Switch Pin | Connector | Type | Length |
|------|------------|-----------|------|--------|
| Port 0 | M14A1 | X1 | SFP+ 10G | < 150 mm |
| Port 1 | M14A2 | X2 | SFP+ 10G | < 150 mm |
| Port 2 | M14A3 | X3 | SFP+ 10G | < 150 mm |
| Port 3 | M14A4 | X4 | SFP+ 10G | < 150 mm |
| Port 4 | M14A5 | X5 | SFP+ 10G | < 150 mm |
| Port 5 | M14A6 | X6 | SFP+ 10G | < 150 mm |
| Port 6 | M14A7 | X7 | SFP+ 10G | < 150 mm |
| Port 7 | M14A8 | X8 | SFP+ 10G | < 150 mm |

### 4.2 88Q5242 ↔ QSFP+ connectors (100G uplink)

| Port | Switch Pin | Connector | Type | Length |
|------|------------|-----------|------|--------|
| Port 8 | N15A1 | Y1 | QSFP+ 100G | < 100 mm |
| Port 9 | N15A2 | Y2 | QSFP+ 100G | < 100 mm |

### 4.3 Ethernet differential pair routing

| Signal | Switch Pin | Connector Pin | Impedance | Length Match |
|--------|------------|---------------|-----------|-------------|
| TX_P | P16A1 | X1-TX1P | 100Ω diff | < 3 mm |
| TX_N | P16A2 | X1-TX1N | 100Ω diff | < 3 mm |
| RX_P | P17A1 | X1-RX1P | 100Ω diff | < 3 mm |
| RX_N | P17A2 | X1-RX1N | 100Ω diff | < 3 mm |

## 5. I2C трассировка

### 5.1 S32G3 ↔ All devices

| Signal | S32G3 Pin | Target | Pull-up | Resistor |
|--------|-----------|--------|---------|----------|
| I2C0_SDA | K14A1 | 88Q5242/EEPROM | 2.2kΩ | R1-R2 |
| I2C0_SCL | K14A2 | 88Q5242/EEPROM | 2.2kΩ | R1-R2 |
| I2C1_SDA | K15A1 | Fan/PSU | 2.2kΩ | R3 |
| I2C1_SCL | K15A2 | Fan/PSU | 2.2kΩ | R3 |

## 6. PTP/1588 трассировка

### 6.1 PTP clock distribution

| Signal | Source | Target | Type | Length |
|--------|--------|--------|------|--------|
| PTP_CLK | 88Q5242 | S32G3 | LVDS | < 10 mm |
| PTP_CLK_N | 88Q5242 | S32G3 | LVDS | < 10 mm |
| PTP_TS | 88Q5242 | S32G3 | LVDS | < 10 mm |

## 7. Power delivery

### 7.1 Power rails

| Rail | Voltage | Current | Regulator | Location |
|------|---------|---------|-----------|----------|
| VCCINT | 0.95V | 8A | TI TPS546D2A | S32G3 |
| VCCDDRO | 0.95V | 4A | TI TPS546D2A | DDR4 |
| VCC AUX | 3.3V | 3A | ON Semi NCP3031 | Edge |
| VBAT | 3.3V | 100mA | MCP1825 | BMC area |

### 7.2 Power sequencing

| Step | Rail | Voltage | Ramp Time | Enable |
|------|------|---------|-----------|--------|
| 1 | VCC AUX | 3.3V | 1ms | PWR_GOOD |
| 2 | VBAT | 3.3V | 0.5ms | PWR_GOOD |
| 3 | VCCINT | 0.95V | 2ms | VCC AUX OK |
| 4 | VCCDDRO | 0.95V | 2ms | VCCINT OK |
| 5 | RESET# | — | — | All rails OK |
