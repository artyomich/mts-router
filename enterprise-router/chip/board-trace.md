# MTS-ER-1000 — Трассировка платы (Main Board)

## 1. Архитектура платы

```
┌─────────────────────────────────────────────────────────────────────┐
│                      MTS-ER-1000 MAIN BOARD                         │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐    │
│  │ NXP      │    │ NXP      │    │ Broadcom │    │ NXP      │    │
│  │ S32G3    │    │ S32G3    │    │ TomTom   │    │ S32G3    │    │
│  │ (CTRL)   │    │ (CTRL)   │    │ (ASIC)   │    │ (MGMT)   │    │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘    │
│       │               │               │               │            │
│  ┌────▼───────────────▼───────────────▼───────────────▼─────┐     │
│  │              DDR4 128 MB + EEPROM 24C64                   │     │
│  └────┬─────────────────────────────────────────────────────┘     │
│       │                                                            │
│  ┌────▼─────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ Fan      │  │ Power    │  │ BMC      │  │ SD Card  │        │
│  │ Ctrl     │  │ Mgmt     │  │ AST2600  │  │ eMMC     │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. PCIe трассировка

### 2.1 S32G3 ↔ TomTom ASIC (PCIe Gen3 x8)

| Lane | S32G3 Pin | TomTom Pin | Impedance | Length |
|------|-----------|------------|-----------|--------|
| PCIe 0 Lane 0 | K12A1 | L23 | 85Ω | < 15 mm |
| PCIe 0 Lane 1 | K12A2 | L24 | 85Ω | < 15 mm |
| PCIe 0 Lane 2 | K12A3 | L25 | 85Ω | < 15 mm |
| PCIe 0 Lane 3 | K12A4 | L26 | 85Ω | < 15 mm |
| PCIe 0 Lane 4 | K12A5 | L27 | 85Ω | < 15 mm |
| PCIe 0 Lane 5 | K12A6 | L28 | 85Ω | < 15 mm |
| PCIe 0 Lane 6 | K12A7 | L29 | 85Ω | < 15 mm |
| PCIe 0 Lane 7 | K12A8 | L30 | 85Ω | < 15 mm |

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
| **Frequency** | 1600 MT/s |
| **VDD** | 1.2 V |
| **VDDQ** | 1.2 V |
| **Impedance** | 40Ω single-ended |
| **Length match** | < 0.5 mm |

## 4. Ethernet port трассировка

### 4.1 TomTom ↔ RJ45/SFP connectors

| Port | TomTom Pin | Connector | Type | Length |
|------|------------|-----------|------|--------|
| GE 1 | M14A1 | X1 | RJ45 1G | < 50 mm |
| GE 2 | M14A2 | X2 | RJ45 1G | < 50 mm |
| GE 3 | M14A3 | X3 | RJ45 1G | < 50 mm |
| GE 4 | M14A4 | X4 | RJ45 1G | < 50 mm |
| WAN 1 | N15A1 | Y1 | SFP+ 10G | < 100 mm |
| WAN 2 | N15A2 | Y2 | SFP+ 10G | < 100 mm |

### 4.2 RJ45 differential pair routing

| Signal | TomTom Pin | RJ45 Pin | Impedance | Length Match |
|--------|------------|----------|-----------|-------------|
| TX_P | P16A1 | J1-1 | 100Ω diff | < 2 mm |
| TX_N | P16A2 | J1-2 | 100Ω diff | < 2 mm |
| RX_P | P17A1 | J1-3 | 100Ω diff | < 2 mm |
| RX_N | P17A2 | J1-4 | 100Ω diff | < 2 mm |

## 5. I2C трассировка

### 5.1 S32G3 ↔ All devices

| Signal | S32G3 Pin | Target | Pull-up | Resistor |
|--------|-----------|--------|---------|----------|
| I2C0_SDA | K14A1 | TomTom/EEPROM | 2.2kΩ | R1-R2 |
| I2C0_SCL | K14A2 | TomTom/EEPROM | 2.2kΩ | R1-R2 |
| I2C1_SDA | K15A1 | Fan/PSU | 2.2kΩ | R3 |
| I2C1_SCL | K15A2 | Fan/PSU | 2.2kΩ | R3 |

## 6. Clock трассировка

### 6.1 Reference clocks

| Clock | Source | Fan-out | Amplitude | Length |
|-------|--------|---------|-----------|--------|
| 125 MHz REF | TomTom | 4 outputs | 800mVpp | < 50 mm |
| 25 MHz REF | TomTom | 2 outputs | 800mVpp | < 50 mm |
| 50 MHz SYS | BMC | 2 outputs | 1.8V | < 50 mm |

## 7. SD/eMMC трассировка

### 7.1 SDIO interface

| Signal | S32G3 Pin | eMMC Pin | Type | Length |
|--------|-----------|----------|------|--------|
| SD_CLK | K16A1 | E1 | 1.8V | < 10 mm |
| SD_CMD | K16A2 | E2 | 1.8V | < 10 mm |
| SD_DAT0 | K16A3 | E3 | 1.8V | < 10 mm |
| SD_DAT1 | K16A4 | E4 | 1.8V | < 10 mm |
| SD_DAT2 | K16A5 | E5 | 1.8V | < 10 mm |
| SD_DAT3 | K16A6 | E6 | 1.8V | < 10 mm |

## 8. Power delivery

### 8.1 Power rails

| Rail | Voltage | Current | Regulator | Location |
|------|---------|---------|-----------|----------|
| VCCINT | 0.95V | 6A | TI TPS546D2A | S32G3 |
| VCCDDRO | 0.95V | 3A | TI TPS546D2A | DDR4 |
| VCC AUX | 3.3V | 2A | ON Semi NCP3031 | Edge |
| VBAT | 3.3V | 100mA | MCP1825 | BMC area |

### 8.2 Power sequencing

| Step | Rail | Voltage | Ramp Time | Enable |
|------|------|---------|-----------|--------|
| 1 | VCC AUX | 3.3V | 1ms | PWR_GOOD |
| 2 | VBAT | 3.3V | 0.5ms | PWR_GOOD |
| 3 | VCCINT | 0.95V | 2ms | VCC AUX OK |
| 4 | VCCDDRO | 0.95V | 2ms | VCCINT OK |
| 5 | RESET# | — | — | All rails OK |
