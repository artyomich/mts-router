# Board Trace: MTS-MC-5000 Mobile Core

## Overview

MTS-MC-5000 Mobile Core Router for 5G EPC/5GC deployment (UPF, SMF, AMF, PCF)
on Marvell ThunderX3 + AMD EPYC + 100G line card architecture.

## Hardware Architecture

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                       MTS-MC-5000 MAIN BOARD                                │
│                                                                              │
│  ┌──────────────────────────┐  ┌──────────────────────────┐                 │
│  │  AMD EPYC 7002 (Naples)  │  │  Marvell ThunderX3       │                 │
│  │  32-Core @ 2.6GHz        │  │  Neoverse V1 ARM64       │                 │
│  │                          │  │  96 cores                  │                 │
│  │  PCIe Gen4 x64 total     │◄─┤  ┌────────────────────┐  │                 │
│  │  DDR4-3200               │  │  │ 128x ARM Cores     │  │                 │
│  │  8x 32GB RDIMM           │  │  │ 256KB L1 I/D       │  │                 │
│  │  256 GB ECC total        │  │  │ 64MB L3 Cache      │  │                 │
│  │  @ 3200MT/s              │  │  │ 100G I/O Subsys    │  │                 │
│  │  Samsung M393A4K40CB1    │  │  │ PCIe 4.0 Root      │  │                 │
│  │                        │  │  └────────────────────┘  │                 │
│  │  NVMe x2 (RAID-1)      │  │         │               │                 │
│  │  2x 3.8 TB Samsung     │  │  ┌─────▼────────┐       │                 │
│  │  PM9A3 ZVTL             │  │  │  ThunderX3   │       │                 │
│  │                        │  │  │  100G MAC    │       │                 │
│  │  BMC IPMI AST2600      │  │  │  64x QSFP28  │       │                 │
│  │  Management            │  │  │  32x SFP28   │       │                 │
│  └────────────────────────┘  │  └──────────────┘       │                 │
│                               │                          │                 │
│  ┌──────────────┐  ┌──────────┴──────────┐  ┌──────────┴──────────┐     │
│  │  PSU 1       │  │  6x Hot-Swap Fans   │  │  PSU 2 (1+1 Red.)  │     │
│  │  400W AC     │  │  Front-to-back      │  │  400W AC           │     │
│  │  12V/33A     │  │  NMB-MAT            │  │  12V/33A           │     │
│  └──────────────┘  └─────────────────────┘  └─────────────────────┘     │
└──────────────────────────────────────────────────────────────────────────────┘
```

## PCIe Tracing

### AMD EPYC ↔ ThunderX3 (PCIe Gen4 x16)

| Lane | CPU Pin | ThunderX3 Pin | Impedance | Length |
|------|---------|---------------|-----------|--------|
| PCIe 0 Lane 0 | J15A1 | K23 | 50Ω | < 15 mm |
| PCIe 0 Lane 1 | J15A2 | K24 | 50Ω | < 15 mm |
| PCIe 0 Lane 2 | J15A3 | K25 | 50Ω | < 15 mm |
| PCIe 0 Lane 3 | J15A4 | K26 | 50Ω | < 15 mm |
| PCIe 0 Lane 4 | J15A5 | K27 | 50Ω | < 15 mm |
| PCIe 0 Lane 5 | J15A6 | K28 | 50Ω | < 15 mm |
| PCIe 0 Lane 6 | J15A7 | K29 | 50Ω | < 15 mm |
| PCIe 0 Lane 7 | J15A8 | K30 | 50Ω | < 15 mm |
| PCIe 0 Lane 8 | J15B1 | K31 | 50Ω | < 15 mm |
| PCIe 0 Lane 9 | J15B2 | K32 | 50Ω | < 15 mm |
| PCIe 0 Lane 10 | J15B3 | K33 | 50Ω | < 15 mm |
| PCIe 0 Lane 11 | J15B4 | K34 | 50Ω | < 15 mm |
| PCIe 0 Lane 12 | J15B5 | K35 | 50Ω | < 15 mm |
| PCIe 0 Lane 13 | J15B6 | K36 | 50Ω | < 15 mm |
| PCIe 0 Lane 14 | J15B7 | K37 | 50Ω | < 15 mm |
| PCIe 0 Lane 15 | J15B8 | K38 | 50Ω | < 15 mm |

### AMD EPYC ↔ NVMe (PCIe Gen4 x4 each)

| Lane | CPU Pin | NVMe Pin | Impedance | Length |
|------|---------|----------|-----------|--------|
| PCIe 1 Lane 0-3 | N18A1-A4 | P20 (M.2) | 50Ω | < 15 mm |
| PCIe 2 Lane 0-3 | N18B1-B4 | P21 (M.2) | 50Ω | < 15 mm |

### BMC ↔ All (PCIe Gen3 x4)

| Lane | BMC Pin | Target | Impedance | Length |
|------|---------|--------|-----------|--------|
| PCIe 3 Lane 0-3 | D20-D23 | EPYC BMC | 50Ω | < 10 mm |

## DDR4 Tracing

### Memory channels (8x 32GB RDIMM)

| Channel | DQ Pins | Address Pins | CLK | Length |
|---------|---------|-------------|-----|--------|
| CH0 | A1-A8 | B1-B4 | C1 | < 5 mm |
| CH1 | A9-A16 | B5-B8 | C2 | < 5 mm |
| CH2 | A17-A24 | B9-B12 | C3 | < 5 mm |
| CH3 | A25-A32 | B13-B16 | C4 | < 5 mm |
| CH4 | A33-A40 | B17-B20 | C5 | < 5 mm |
| CH5 | A41-A48 | B21-B24 | C6 | < 5 mm |
| CH6 | A49-A56 | B25-B28 | C7 | < 5 mm |
| CH7 | A57-A64 | B29-B32 | C8 | < 5 mm |

### DDR4 timing parameters

| Parameter | Value |
|-----------|-------|
| Frequency | 3200 MT/s |
| VDD | 1.2 V |
| VDDQ | 1.2 V |
| Impedance | 40Ω differential |
| Length match | < 0.5 mm |
| ECC | Enabled (72-bit per DIMM) |

## I2C Tracing

### BMC ↔ All devices

| Signal | BMC Pin | Target | Pull-up |
|--------|---------|--------|---------|
| SDA0 | D30 | ThunderX3 PMU | 4.7kΩ |
| SCL0 | D31 | ThunderX3 PMU | 4.7kΩ |
| SDA1 | D32 | BMC IPMI | 4.7kΩ |
| SCL1 | D33 | BMC IPMI | 4.7kΩ |
| SDA2 | D34 | Power Mgmt | 4.7kΩ |
| SCL2 | D35 | Power Mgmt | 4.7kΩ |
| SDA3 | D36 | Thermal | 4.7kΩ |
| SCL3 | D37 | Thermal | 4.7kΩ |
| SDA4 | D38 | EEPROM | 4.7kΩ |
| SCL4 | D39 | EEPROM | 4.7kΩ |

## SPI Tracing

### BMC ↔ SPI Flash

| Signal | BMC Pin | Flash Pin | Length |
|--------|---------|-----------|--------|
| MOSI | E40 | PIN 5 | < 5 mm |
| MISO | E41 | PIN 2 | < 5 mm |
| SCK | E42 | PIN 6 | < 5 mm |
| CS | E43 | PIN 4 | < 5 mm |

## Line Card Tracing

### ThunderX3 ↔ 100G QSFP28 Ports

| Port | SerDes Lane | Pin | Impedance | Length |
|------|-------------|-----|-----------|--------|
| QSFP0 Lane 0-3 | SerDes 0-3 | F20-F23 | 50Ω | < 20 mm |
| QSFP0 Lane 4-7 | SerDes 4-7 | F24-F27 | 50Ω | < 20 mm |
| QSFP1 Lane 0-3 | SerDes 8-11 | F28-F31 | 50Ω | < 20 mm |
| QSFP1 Lane 4-7 | SerDes 12-15 | F32-F35 | 50Ω | < 20 mm |
| ... | ... | ... | ... | ... |
| QSFP31 Lane 0-3 | SerDes 124-127 | G20-G23 | 50Ω | < 20 mm |

## PCB Parameters

| Parameter | Value |
|-----------|-------|
| Layers | 20-layer |
| Material | Rogers RO4350B (high freq) |
| Substrate | FR-4 (low freq) |
| Impedance | 50Ω single, 100Ω differential |
| Copper | 1 oz (outer), 0.5 oz (inner) |
| Surface | ENIG (gold finish) |
| Via | Microvia (0.15 mm), Blind (0.3 mm) |
| Thickness | 4.8 mm (1RU form factor) |
| Signal Integrity | SI-validated < 16 GT/s |
| Power Integrity | PI-validated 256 GB DDR4 |

## Power Distribution

### Voltage Rails

| Rail | Voltage | Current | Regulator |
|------|---------|---------|-----------|
| V_CORE_EPYC | 0.8V | 500A | VICOR VI-200 |
| V_DDR4 | 1.2V | 80A | TI TPS546D24A |
| V_PCIE | 3.3V | 50A | TI LM53600 |
| V_AUX | 3.3V | 20A | ON Semi NCP3031 |
| V_FAN | 12V | 6A | Direct |

### Power sequencing

1. 3.3V AUX (standby)
2. 3.3V PCIe
3. 1.2V DDR4
4. 0.8V EPYC core
5. 1.2V ThunderX3 core
6. Boot complete (Power Good asserted)
