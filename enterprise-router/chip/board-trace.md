# Board Trace: MTS-ER-1000 Enterprise Router

## Overview

MTS-ER-1000 Enterprise Router для корпоративного сегмента (SD-WAN, IPsec, FRRouting)
на базе NXP S32G + Broadcom TomTom ASIC.

## Hardware Architecture

```
┌────────────────────────────────────────────────────────────────────────────┐
│                      MTS-ER-1000 MAIN BOARD                               │
│                                                                            │
│  ┌──────────────────────┐  ┌──────────────────────────┐                   │
│  │  NXP S32G            │  │  Broadcom TomTom ASIC    │                   │
│  │  (Cortex-A53 x4)     │  │  (TRI-QUAD forwarding)   │                   │
│  │  @ 1.8GHz            │  │                          │                   │
│  │                      │  │  ┌────────────────────┐  │                   │
│  │  DDR4-2666           │  │  │ 12x 10G SerDes     │  │                   │
│  │  1GB ECC             │  │  │ 4x 25G SerDes      │  │                   │
│  │  @ 2666MT/s          │  │  │ 8x 1G RGMII        │  │                   │
│  │                      │  │  │ Hardware IPsec     │  │                   │
│  │  eTPU x2             │  │  │ TCAM 128K entries  │  │                   │
│  │  FlexCAN x4          │  │  │ BFD/ECMP hardware  │  │                   │
│  │  USB 3.0 / 2.0       │  │  │ SRv6 engine        │  │                   │
│  └──────────┬───────────┘  └──────────────────────────┘                   │
│             │  PCIe / I2C / MDIO                                          │
│             │                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐            │
│  │  EEPROM      │  │  Crystal     │  │  Power               │            │
│  │  24AA02E64   │  │  125MHz      │  │  TPS54020            │            │
│  │  I2C addr    │  │  TCXO        │  │  5V → 5V/3.3V/1.0V │            │
│  └──────────────┘  └──────────────┘  └──────────────────────┘            │
│                                                                            │
│  ┌────────────────────────────────────────────────────────────────────┐    │
│  │                    Port Panel                                      │    │
│  │                                                                    │    │
│  │  SFP+1  SFP+2  SFP+3  SFP+4  SFP+5  SFP+6                       │    │
│  │  SFP+7  SFP+8  RJ45-1  RJ45-2  RJ45-3  RJ45-4                    │    │
│  │    │      │      │      │      │      │      │      │             │    │
│  │    └───────┴──────┴──────┴──────┴──────┴──────┴──────┘            │    │
│  │                    │                                                │    │
│  │            TomTom ASIC Ports                                       │    │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐            │
│  │  TPM 2.0     │  │  Thermal     │  │  Management          │            │
│  │  Infineon    │  │  NXP TMP461  │  │  USB Console         │            │
│  │  I2F75       │  │  I2C addr    │  │  RJ45 MGMT           │            │
│  └──────────────┘  └──────────────┘  └──────────────────────┘            │
└────────────────────────────────────────────────────────────────────────────┘
```

## PCIe Tracing

### S32G ↔ TomTom Connection

```
NXP S32G PCIe Root Complex
│
├── Lane 0-3 (x4) → TomTom ASIC (PCIe Gen3)
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

## MDIO Tracing (S32G ↔ TomTom)

```
S32G MDIO0
│
├── MDIO0_SDA → TomTom ASIC (MDIO Slave)
├── MDIO0_SCLK → TomTom ASIC
├── MDIO1_SDA → TomTom ASIC (backup)
└── MDIO1_SCLK → TomTom ASIC
```

## DDR4 Tracing

### Memory Layout

```
S32G Memory Controller
│
├── Channel 0 (single channel DDR4)
│   ├── DQ[0:15]     →  U14 (Micron MT52L256M32D1DI)
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

## I2C Tracing

### Management Bus

```
S32G I2C0 (BMC)
│
├── SDA0 → EEPROM 24AA02E64 (addr 0x50)
├── SCL0 → EEPROM
├── SDA0 → Thermal TMP461 (addr 0x4C)
├── SCL0 → Thermal
├── SDA0 → TomTom ASIC (addr 0x58)
├── SCL0 → TomTom
├── SDA0 → TPM 2.0 I2F75 (addr 0x5A)
├── SCL0 → TPM
├── SDA0 → PSU Controller (addr 0x34)
└── SCL0 → PSU
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
3. 1.0V CORE (NXP PMIC, enabled by BMC)
   │
   ├── Powers S32G core
   └── Powers TomTom core
   │
4. 1.2V DDR (NXP PMIC, enabled by BMC)
   │
   └── Powers DDR4 VDD/VDDQ
   │
5. PCIe PERST# (deasserted by BMC)
   │
   └── TomTom ASIC initializes
```

## Signal Integrity Notes

1. **PCIe**: All lanes length-matched within 5 mils, 100Ω differential impedance
2. **DDR4**: All DQ lines matched within 15 mils, 40Ω single-ended
3. **Reference clocks**: 125MHz TCXO, ±10ppm stability
4. **Power planes**: 6-layer minimum, split analog/digital
5. **Via stubs**: Back-drilled for PCIe > 5 Gbps
6. **Termination**: On-die termination (ODT) enabled for DDR4
7. **SerDes**: Controlled impedance for 10G/25G differential pairs

## Clock Distribution

```
125MHz TCXO (Abracon ABM8-125.0MHz)
│
├── Output 1 → S32G REFCLK0
├── Output 2 → S32G REFCLK1
├── Output 3 → TomTom ASIC REFCLK (125MHz)
└── Output 4 → Reserved
```

## Port Mapping

| Port | Type | Speed | Description |
|------|------|-------|-------------|
| SFP+1-8 | SFP+ | 10G | 8x 10G SFP+ ports |
| RJ45-1-4 | RJ45 | 1G | 4x Gigabit Ethernet |
| MGMT | RJ45 | 1G | Management |
| CONSOLE | USB | 12M | Console |

## BOM Highlights

| Component | Part | Qty | Supplier |
|-----------|------|-----|----------|
| S32G | NXP S32G274A | 1 | DigiKey |
| TomTom | Broadcom BCM56840 | 1 | Avnet |
| DDR4 | Micron MT52L256M32D1DI | 1 | Mouser |
| EEPROM | Microchip 24AA02E64 | 1 | DigiKey |
| PMIC | NXP FXOS5807 | 1 | Avnet |
| Thermal | NXP TMP461 | 1 | DigiKey |
| TPM | Infineon I2F75 | 1 | Mouser |
| TCXO | Abracon ABM8-125.0MHz | 1 | DigiKey |
| Power IC | TI TPS54020 | 2 | Mouser |
