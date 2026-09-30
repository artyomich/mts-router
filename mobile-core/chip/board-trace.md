# MTS-MC-5000 — Трассировка платы (Main Board)

## 1. Архитектура платы

```
┌─────────────────────────────────────────────────────────────────────┐
│                       MTS-MC-5000 MAIN BOARD                       │
├─────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────┐           │
│  │  Marvell ThunderX3 ARM64 CPU                        │           │
│  │  96 cores Neoverse V1 @ 2.2 GHz                    │           │
│  │  PCIe 5.0 x16 / CXL 2.0 / DDR5                     │           │
│  └────┬───────────────────────────────────┬───────────┘           │
│       │                                   │                        │
│  ┌────▼─────┐     ┌────────────────────┐  ┌──────────────────┐   │
│  │ DDR5     │     │ Marvell            │  │ BMC              │   │
│  │ 256 GB   │     │ 88Q5242            │  │ IPMI             │   │
│  │ 4x 64GB  │     │ (100G Switch)      │  │ ASPEED AST2600   │   │
│  └──────────┘     └────────────────────┘  └──────────────────┘   │
│  ┌──────────┐     ┌────────────────────┐  ┌──────────────────┐   │
│  │ NVMe     │     │ CPLD               │  │ Power            │   │
│  │ 3.8 TB   │     │ EPM240             │  │ Management       │   │
│  └──────────┘     └────────────────────┘  └──────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. PCIe трассировка

### 2.1 CPU ↔ 88Q5242 Switch (PCIe Gen5 x8)

| Lane | CPU Pin | Switch Pin | Impedance | Length |
|------|---------|------------|-----------|--------|
| PCIe 0 Lane 0 | J1A1 | K23 | 85Ω | < 20 mm |
| PCIe 0 Lane 1 | J1A2 | K24 | 85Ω | < 20 mm |
| PCIe 0 Lane 2 | J1A3 | K25 | 85Ω | < 20 mm |
| PCIe 0 Lane 3 | J1A4 | K26 | 85Ω | < 20 mm |
| PCIe 0 Lane 4 | J1A5 | K27 | 85Ω | < 20 mm |
| PCIe 0 Lane 5 | J1A6 | K28 | 85Ω | < 20 mm |
| PCIe 0 Lane 6 | J1A7 | K29 | 85Ω | < 20 mm |
| PCIe 0 Lane 7 | J1A8 | K30 | 85Ω | < 20 mm |

### 2.2 CPU ↔ BMC (PCIe Gen3 x4)

| Lane | CPU Pin | BMC Pin | Impedance | Length |
|------|---------|---------|-----------|--------|
| PCIe 1 Lane 0 | J2A1 | M10 | 85Ω | < 15 mm |
| PCIe 1 Lane 1 | J2A2 | M11 | 85Ω | < 15 mm |
| PCIe 1 Lane 2 | J2A3 | M12 | 85Ω | < 15 mm |
| PCIe 1 Lane 3 | J2A4 | M13 | 85Ω | < 15 mm |

### 2.3 CPU ↔ NVMe (PCIe Gen5 x4)

| Lane | CPU Pin | NVMe Pin | Impedance | Length |
|------|---------|----------|-----------|--------|
| PCIe 2 Lane 0 | J3A1 | P20 | 85Ω | < 20 mm |
| PCIe 2 Lane 1 | J3A2 | P21 | 85Ω | < 20 mm |
| PCIe 2 Lane 2 | J3A3 | P22 | 85Ω | < 20 mm |
| PCIe 2 Lane 3 | J3A4 | P23 | 85Ω | < 20 mm |

## 3. DDR5 трассировка

### 3.1 Memory channels

| Channel | DQ Pins | Address Pins | CLK | Length |
|---------|---------|-------------|-----|--------|
| CH0 | A1-A8 | B1-B4 | C1 | < 8 mm |
| CH1 | A9-A16 | B5-B8 | C2 | < 8 mm |
| CH2 | A17-A24 | B9-B12 | C3 | < 8 mm |
| CH3 | A25-A32 | B13-B16 | C4 | < 8 mm |

### 3.2 DDR5 timing

| Parameter | Value |
|-----------|-------|
| **Frequency** | 4800 MT/s |
| **VDD** | 1.1 V |
| **VDDQ** | 1.1 V |
| **Impedance** | 40Ω single-ended |
| **Length match** | < 1.0 mm |
| **Termination** | DFX 40Ω |

## 4. Ethernet port трассировка

### 4.1 88Q5242 ↔ SFP28 connectors (100G ports)

| Port | Switch Pin | Connector | Type | Length |
|------|------------|-----------|------|--------|
| Port 0 | L12A1 | X1 | SFP28 100G | < 100 mm |
| Port 1 | L12A2 | X2 | SFP28 100G | < 100 mm |
| Port 2 | L12A3 | X3 | SFP28 100G | < 100 mm |
| Port 3 | L12A4 | X4 | SFP28 100G | < 100 mm |
| Port 4 | L12A5 | X5 | SFP28 100G | < 100 mm |
| Port 5 | L12A6 | X6 | SFP28 100G | < 100 mm |
| Port 6 | L12A7 | X7 | SFP28 100G | < 100 mm |
| Port 7 | L12A8 | X8 | SFP28 100G | < 100 mm |

### 4.2 Ethernet differential pair routing

| Signal | Switch Pin | Connector Pin | Impedance | Length Match |
|--------|------------|---------------|-----------|-------------|
| TX_P | M14A1 | X1-TX1P | 100Ω diff | < 2 mm |
| TX_N | M14A2 | X1-TX1N | 100Ω diff | < 2 mm |
| RX_P | M15A1 | X1-RX1P | 100Ω diff | < 2 mm |
| RX_N | M15A2 | X1-RX1N | 100Ω diff | < 2 mm |

## 5. I2C трассировка

### 5.1 BMC ↔ All devices

| Signal | BMC Pin | Target | Pull-up | Resistor |
|--------|---------|--------|---------|----------|
| I2C_SDA | N14A1 | 88Q5242/EEPROM/PSU | 4.7kΩ | R1-R3 |
| I2C_SCL | N14A2 | 88Q5242/EEPROM/PSU | 4.7kΩ | R1-R3 |
| SMB_ALERT | N14A3 | PSU/Fan | 4.7kΩ | R4 |

## 6. Clock трассировка

### 6.1 Reference clocks

| Clock | Source | Fan-out | Amplitude | Length |
|-------|--------|---------|-----------|--------|
| 125 MHz REF | 88Q5242 | 8 outputs | 800mVpp | < 50 mm |
| 25 MHz REF | 88Q5242 | 4 outputs | 800mVpp | < 50 mm |
| 1588 PTP | BMC | 2 outputs | 1.8V | < 50 mm |

## 7. Power delivery

### 7.1 Power rails

| Rail | Voltage | Current | Regulator | Location |
|------|---------|---------|-----------|----------|
| VCCINT | 0.8V | 120A | TI TPS546D2A | Center |
| VCCDDRO | 0.8V | 40A | TI TPS546D2A | CH0-CH3 |
| VCC AUX | 3.3V | 5A | ON Semi NCP3031 | Edge |
| VBAT | 3.3V | 100mA | MCP1825 | BMC area |

### 7.2 Power sequencing

| Step | Rail | Voltage | Ramp Time | Enable |
|------|------|---------|-----------|--------|
| 1 | VCC AUX | 3.3V | 1ms | PWR_GOOD |
| 2 | VBAT | 3.3V | 0.5ms | PWR_GOOD |
| 3 | VCCINT | 0.8V | 2ms | VCC AUX OK |
| 4 | VCCDDRO | 0.8V | 2ms | VCCINT OK |
| 5 | PCIe PERST# | — | — | All rails OK |
