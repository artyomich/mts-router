# Board Trace: MTS-MB-3000 Mobile Backhaul

## Overview

MTS-MB-3000 Mobile Backhaul Router for 5G small cell backhaul (MPLS-TP, PTP, SyncE)
на базе NXP S32G3 + Marvell 88Q5242.

## Hardware Architecture

```
┌────────────────────────────────────────────────────────────────────────────┐
│                       MTS-MB-3000 MAIN BOARD                              │
│                                                                            │
│  ┌──────────────────────────┐  ┌──────────────────────────┐               │
│  │  NXP S32G3               │  │  Marvell 88Q5242         │               │
│  │  (Cortex-A53 x4)         │  │  6-Port 10G Ethernet     │               │
│  │  @ 1.8GHz                │  │  Switch                   │               │
│  │                          │  │                           │               │
│  │  DDR4-3200               │  │  QSFP28 x2 (10G each)   │               │
│  │  2GB ECC                 │  │  SFP+ x4 (10G each)     │               │
│  │  @ 3200MT/s              │  │  SFP x8 (1G each)       │               │
│  │                          │  │                           │               │
│  │  CAN-FD x2               │  │  MDIO management         │               │
│  │  FlexCAN x4              │  │  PTP transparent clock   │               │
│  │  eTPU x2                 │  │                           │               │
│  │  Ethernet MAC x4         │  │  QSFP0 ── 10G uplink    │               │
│  │  USB 3.0 / 2.0           │  │  QSFP1 ── 10G uplink    │               │
│  │  PCIe 3.0                │  │  SFP+2-5 ── 10G ports   │               │
│  └──────────┬───────────────┘  │  SFP6-7 ── 1G ports     │               │
│             │                   └──────────────────────────┘               │
│             │ I2C / MDIO / PCIe                                  ┌────────┴────────┐
│             │                                                                    │ 10G PHYs     │
│             │                                                                    │ 88Q5242      │
│             └────────────────────────────────────────────────────────────────────┘                │
│                                                                                                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  ┌─────────────────────────────┐ │
│  │  EEPROM      │  │  Crystal     │  │  Power               │  │  Thermal                   │ │
│  │  24AA02E64   │  │  125MHz      │  │  TPS54020            │  │  NXP TMP461                │ │
│  │  I2C addr    │  │  TCXO        │  │  5V → 5V/3.3V/1.0V │  │  I2C addr 0x4C             │ │
│  └──────────────┘  └──────────────┘  └──────────────────────┘  └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────────────────────────┘
```

## PCIe Tracing

### S32G3 ↔ 88Q5242 Connection

```
NXP S32G3 PCIe Root Complex
│
├── Lane 0-3 (x4) → 88Q5242 (PCIe Gen3)
│   │
│   ├── Signal: PCIe_TX_P[0:3]
│   ├── Signal: PCIe_TX_N[0:3]
│   ├── Signal: PCIe_RX_P[0:3]
│   ├── Signal: PCIe_RX_N[0:3]
│   ├── Signal: PCIe_REF_CLK_P (100MHz)
│   ├── Signal: PCIe_REF_CLK_N (100MHz)
│   ├── Signal: PCIe_PERST_N
│   ├── Impedance: 100Ω differential
│   ├── Length: < 0.5 inch (direct)
│   ├── Termination: 100Ω diff on PCB
│   └── Speed: PCIe Gen3 x4 = 16 Gbps
│
└── Reserved lanes (unused)
    ├── PCIe_EXT_TX_P/N[0:15]
    └── PCIe_EXT_RX_P/N[0:15]
```

## MDIO Tracing (S32G3 ↔ 88Q5242)

```
S32G3 MDIO0
│
├── MDIO0_SDA → 88Q5242 MDC_MDIO (pin 1)
├── MDIO0_SCLK → 88Q5242 MDC_MDIO (pin 2)
├── MDIO1_SDA → 88Q5242 MDC_MDIO (pin 3)
└── MDIO1_SCLK → 88Q5242 MDC_MDIO (pin 4)
```

## DDR4 Tracing

### Memory Layout

```
S32G3 Memory Controller
│
├── Channel 0 (single channel DDR4)
│   ├── DQ[0:15]     →  U14 (Micron MT52L256)
│   ├── Address/Command [0:17]
│   ├── Clock P/N    →  U14 CLK
│   ├── VDD          →  1.2V
│   ├── VDDQ         →  1.2V
│   ├── VPP          →  1.8V (VREFCA)
│   ├── Impedance: 40Ω single-ended
│   ├── Timing: tRC=55ns, tRAS=33ns
│   └── Length: < 0.5 inch skew
│
└── ECC bits (8 extra)
    ├── DQ[16:23]    →  U14 ECC
    └── VDD          →  1.2V
```

### DDR4 Timing Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| fCLK | 1600 MHz | Memory clock frequency |
| tRC | 55 ns | Row to Column delay |
| tRAS | 33 ns | Active to Precharge |
| tRCD | 13.75 ns | Row to Column delay |
| tWR | 15 ns | Write Recovery |
| tRP | 13.75 ns | Row Precharge |
| tRRD | 4.6875 ns | Row to Row delay |
| tFAW | 25 ns | Four Activate Window |
| Impedance | 40Ω | Single-ended |

## I2C Tracing

### Management Bus

```
S32G3 I2C0 (BMC)
│
├── SDA0 → EEPROM 24AA02E64 (addr 0x50)
├── SCL0 → EEPROM
├── SDA0 → Thermal TMP461 (addr 0x4C)
├── SCL0 → Thermal
├── SDA0 → 88Q5242 MDIO (addr 0x58)
├── SCL0 → 88Q5242
└── SDA0 → RTC (NXP PCF85063, addr 0x51)
    └── SCL0 → RTC
```

## Power Sequence

```
1. 5V EXT (from external PSU)
   │
   ├── Powers 5V rail
   └── Powers RJ45 magnetics
   │
2. 3.3V AUX (TPS54020, always on)
   │
   ├── Powers EEPROM
   ├── Powers I/O
   └── Powers PCIe refclk
   │
3. 5V_SW (TPS54020, enabled by PMIC)
   │
   ├── Powers SFP modules
   └── Powers QSFP modules
   │
4. 1.0V CORE (NXP PMIC, enabled by BMC)
   │
   ├── Powers S32G3 core
   └── Powers 88Q5242 core
   │
5. 1.2V DDR (NXP PMIC, enabled by BMC)
   │
   └── Powers DDR4 VDD/VDDQ
   │
6. PCIe PERST# (deasserted by BMC)
   │
   └── 88Q5242 initializes
```

## Signal Integrity Notes

1. **PCIe**: All lanes length-matched within 5 mils, 100Ω differential impedance
2. **DDR4**: All DQ lines matched within 15 mils, 40Ω single-ended
3. **Reference clocks**: 125MHz TCXO, ±10ppm stability
4. **Power planes**: 6-layer minimum, split analog/digital
5. **Via stubs**: Back-drilled for PCIe > 5 Gbps
6. **Termination**: On-die termination (ODT) enabled for DDR4
7. **SFP/QSFP**: Controlled impedance traces for 10G PAM3

## Clock Distribution

```
125MHz TCXO (Abracon ABM8-125.0MHz)
│
├── Output 1 → S32G3 REFCLK0
├── Output 2 → S32G3 REFCLK1
├── Output 3 → 88Q5242 REFCLK (125MHz)
├── Output 4 → SFP+ clock reference
└── Output 5 → QSFP clock reference
```

## BOM Highlights

| Component | Part | Qty | Supplier |
|-----------|------|-----|----------|
| S32G3 | NXP S32G396RVM | 1 | DigiKey |
| 88Q5242 | Marvell 88Q5242-B0 | 1 | Avnet |
| DDR4 | Micron MT52L256M32D1DI | 1 | Mouser |
| EEPROM | Microchip 24AA02E64 | 1 | DigiKey |
| PMIC | NXP FXOS5807 | 1 | Avnet |
| Thermal | NXP TMP461 | 1 | DigiKey |
| RTC | NXP PCF85063 | 1 | Mouser |
| TCXO | Abracon ABM8-125.0MHz | 1 | DigiKey |
| Power IC | TI TPS54020 | 2 | Mouser |

## Port Mapping

| Port | Type | Speed | PHY | Description |
|------|------|-------|-----|-------------|
| QSFP0 | QSFP28 | 10G/40G | 88Q5242 Port 0 | Uplink A |
| QSFP1 | QSFP28 | 10G/40G | 88Q5242 Port 1 | Uplink B |
| SFP+2 | SFP+ | 10G | 88Q5242 Port 2 | Backhaul 1 |
| SFP+3 | SFP+ | 10G | 88Q5242 Port 3 | Backhaul 2 |
| SFP+4 | SFP+ | 10G | 88Q5242 Port 4 | Backhaul 3 |
| SFP+5 | SFP+ | 10G | 88Q5242 Port 5 | Backhaul 4 |
| SFP6 | SFP | 1G | 88Q5242 Port 6 | Management |
| SFP7 | SFP | 1G | 88Q5242 Port 7 | Spare |
