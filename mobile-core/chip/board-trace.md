# Board Trace: MTS-MC-5000 Mobile Core

## Overview

MTS-MC-5000 Mobile Core Router — высокопроизводительный маршрутизатор для 5G Core (UPF/SMF/AMF/PCF)
на базе Marvell ThunderX3 + AMD EPYC.

## Hardware Architecture

```
┌────────────────────────────────────────────────────────────────────────────┐
│                        MTS-MC-5000 MAIN BOARD                             │
│                                                                            │
│  ┌──────────────┐    ┌──────────────────────────────┐    ┌──────────────┐ │
│  │  AMD EPYC    │    │      ThunderX3 CN9180        │    │   DDR5       │ │
│  │  7002 Series │    │   (ARM Neoverse V1 x8)       │    │  4x 32GB     │ │
│  │  64-Core     │    │   5nm PCIe 4.0 Switch        │    │  5120MT/s    │ │
│  │  @ 2.8GHz    │    │                              │    │  (1TB total)   │ │
│  │              │    │   ┌─────┐ ┌─────┐ ┌─────┐   │    │              │ │
│  │  PCIe Gen4   │───►│   │ Lane│ │ Lane│ │ Lane│   │    │  DDR5-5120   │ │
│  │  x128 total  │    │   │ 0-15│ │16-31│ │32-47│   │    │  Samsung     │ │
│  │              │    │   └─────┘ └─────┘ └─────┘   │    │  M471A4     │ │
│  │  DDR5 ECC    │    │         ┌─────┐              │    │  K3F6         │ │
│  │  8x channels │    │         │ Lane│              │    │                │ │
│  │  @ 4800MT/s  │    │         │48-63│              │    │                │ │
│  └──────────────┘    │         └─────┘              │    └──────────────┘ │
│                      │         ┌─────┐              │                     │
│                      │         │ Lane│              │    ┌──────────────┐ │
│                      │         │64-79│              │    │  NVMe SSD    │ │
│                      │         └─────┘              │    │  2TB         │ │
│                      │         ┌─────┐              │    │  Samsung     │ │
│                      │         │ Lane│              │    │  PM9A3        │ │
│                      │         │80-95│              │    │  ZT1TL        │ │
│                      │         └─────┘              │    └──────────────┘ │
│                      └──────────────────────────────┘                     │
│                                                                            │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                    16x 100G QSFP28 Ports                            │  │
│  │                                                                       │  │
│  │  QSFP0  QSFP1  QSFP2  QSFP3  QSFP4  QSFP5  QSFP6  QSFP7            │  │
│  │  QSFP8  QSFP9  QSFP10 QSFP11 QSFP12 QSFP13 QSFP14 QSFP15            │  │
│  │    │     │      │      │      │      │      │      │                 │  │
│  │    └──────┴──────┴──────┴──────┴──────┴──────┴──────┘                 │  │
│  │                     │                                                   │  │
│  │              ThunderX3 Port Controllers                                 │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐           │
│  │  BMC         │  │  TPM 2.0     │  │  Management          │           │
│  │  IPMI 2.0    │  │  Infineon    │  │  Ethernet (MGMT)     │           │
│  │  ASPEED      │  │  I2F75       │  │  1G RJ45              │           │
│  └──────────────┘  └──────────────┘  └──────────────────────┘           │
└────────────────────────────────────────────────────────────────────────────┘
```

## PCIe Tracing

### CPU ↔ ThunderX3 Connection

```
AMD EPYC 7002 (PCIe Root Complex)
│
├── Lane 0-15  →  ThunderX3 CN9180 Slot 0 (x16)
│   │
│   ├── Signal: PCIe_P0A_TX_P[0:15]
│   ├── Signal: PCIe_P0A_TX_N[0:15]
│   ├── Signal: PCIe_P0A_RX_P[0:15]
│   ├── Signal: PCIe_P0A_RX_N[0:15]
│   ├── Signal: PCIe_P0A_REF_CLK_P
│   ├── Signal: PCIe_P0A_REF_CLK_N
│   ├── Signal: PCIe_P0A_PERST_N
│   ├── Impedance: 100Ω differential
│   ├── Length: < 1 inch (direct)
│   └── Termination: 100Ω diff on PCB
│
├── Lane 16-31 →  ThunderX3 CN9180 Slot 0 (x16, continued)
│   ├── Signal: PCIe_P0B_TX_P[0:15]
│   ├── Signal: PCIe_P0B_TX_N[0:15]
│   ├── Signal: PCIe_P0B_RX_P[0:15]
│   ├── Signal: PCIe_P0B_RX_N[0:15]
│   ├── Impedance: 100Ω differential
│   └── Length: < 1 inch
│
├── Lane 32-47 →  ThunderX3 CN9180 Slot 1 (x16)
│   ├── Signal: PCIe_P1A_TX_P[0:15]
│   ├── Signal: PCIe_P1A_TX_N[0:15]
│   ├── Signal: PCIe_P1A_RX_P[0:15]
│   ├── Signal: PCIe_P1A_RX_N[0:15]
│   ├── Impedance: 100Ω differential
│   └── Length: < 1.5 inch
│
├── Lane 48-63 →  ThunderX3 CN9180 Slot 1 (x16, continued)
│   ├── Signal: PCIe_P1B_TX_P[0:15]
│   ├── Signal: PCIe_P1B_TX_N[0:15]
│   ├── Signal: PCIe_P1B_RX_P[0:15]
│   ├── Signal: PCIe_P1B_RX_N[0:15]
│   ├── Impedance: 100Ω differential
│   └── Length: < 1.5 inch
│
├── Lane 64-79 →  NVMe SSD (M.2 Key M)
│   ├── Signal: PCIe_M2_TX_P[0:15]
│   ├── Signal: PCIe_M2_TX_N[0:15]
│   ├── Signal: PCIe_M2_RX_P[0:15]
│   ├── Signal: PCIe_M2_RX_N[0:15]
│   ├── Impedance: 100Ω differential
│   └── Length: < 2 inch (M.2 form factor)
│
└── Lane 80-95 →  Reserved / Expansion
    ├── Signal: PCIe_EXT_TX_P[0:15]
    ├── Signal: PCIe_EXT_TX_N[0:15]
    ├── Signal: PCIe_EXT_RX_P[0:15]
    ├── Signal: PCIe_EXT_RX_N[0:15]
    └── Length: < 3 inch
```

## DDR5 Tracing

### Memory Channel Layout

```
AMD EPYC 7002 Memory Controller
│
├── Channel 0 (DIMM0)
│   ├── DQ[0:63]     →  U5 (Samsung M471A4)
│   ├── Address/Command [0:11]
│   ├── Clock P/N    →  U5 CLK
│   ├── VDD/PDB      →  1.1V
│   ├── VDDQ/PDQ     →  1.1V
│   ├── Impedance: 40Ω single-ended
│   └── Timing: tRC=67.5ns, tRAS=36ns
│
├── Channel 1 (DIMM1)
│   ├── DQ[0:63]     →  U7 (Samsung M471A4)
│   ├── Timing: same as Ch0
│   └── Length matched: < 0.5 inch skew
│
├── Channel 2 (DIMM2)
│   ├── DQ[0:63]     →  U9 (Samsung M471A4)
│   └── Timing: same as Ch0
│
├── Channel 3 (DIMM3)
│   ├── DQ[0:63]     →  U11 (Samsung M471A4)
│   └── Timing: same as Ch0
│
├── Channel 4 (DIMM4)
│   ├── DQ[0:63]     →  U13 (Samsung M471A4)
│   └── Timing: same as Ch0
│
├── Channel 5 (DIMM5)
│   ├── DQ[0:63]     →  U15 (Samsung M471A4)
│   └── Timing: same as Ch0
│
├── Channel 6 (DIMM6)
│   ├── DQ[0:63]     →  U17 (Samsung M471A4)
│   └── Timing: same as Ch0
│
└── Channel 7 (DIMM7)
    ├── DQ[0:63]     →  U19 (Samsung M471A4)
    └── Timing: same as Ch0
```

### DDR5 Timing Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| fCLK | 2400 MHz | Memory clock frequency |
| tRC | 67.5 ns | Row to Column delay |
| tRAS | 36 ns | Active to Precharge |
| tRCD | 36 ns | Row to Column delay |
| tWR | 15 ns | Write Recovery |
| tRP | 36 ns | Row Precharge |
| tRRD | 6 ns | Row to Row delay |
| tFAW | 30 ns | Four Activate Window |
| Impedance | 40Ω | Single-ended |
| Diff Impedance | N/A | DDR is single-ended |

## I2C Tracing

### BMC Management Bus

```
BMC (ASPEED AST2600) I2C0
│
├── SDA0 → ThunderX3 CN9180 (I2C Slave, addr 0x5C)
├── SCL0 → ThunderX3 CN9180
├── SDA0 → TPM 2.0 (Infineon I2F75, addr 0x5A)
├── SCL0 → TPM 2.0
├── SDA0 → PSU Controller (addr 0x34)
├── SCL0 → PSU Controller
├── SDA0 → Thermal Sensor (TI TMP468, addr 0x4C/4D/4E/4F)
├── SCL0 → Thermal Sensor
├── SDA0 → VCC_PWR (Power Sequencer, TI TPS546D2A, addr 0x32)
├── SCL0 → VCC_PWR
└── SDA0 → PMIC (NXP FXOS5807, addr 0x21)
    └── SCL0 → PMIC
```

## Power Sequence

```
1. 3.3V AUX (always on from PSU)
   │
   ├── Powers BMC standby
   ├── Powers TPM
   └── Powers RTC
   │
2. 3.3V MAIN (enabled by BMC)
   │
   ├── Powers DDR VDD/PDQ
   ├── Powers I/O
   └── Powers PCIe refclk
   │
3. VCC_CORE (1.0V, enabled by BMC)
   │
   ├── Powers EPYC core
   └── Powers ThunderX3 core
   │
4. VCC_DDR (1.1V, enabled by BMC)
   │
   └── Powers DDR VDDQ
   │
5. PCIe PERST# (deasserted by BMC)
   │
   ├── EPYC PCIe lanes come up
   ├── ThunderX3 initializes
   └── NVMe powers on
```

## Signal Integrity Notes

1. **PCIe**: All lanes length-matched within 5 mils, 100Ω differential impedance
2. **DDR5**: All DQ lines matched within 20 mils, 40Ω single-ended
3. **Reference clocks**: 100MHz, length-matched within 5 mils
4. **Power planes**: 4-layer minimum, split analog/digital
5. **Via stubs**: Back-drilled for PCIe > 5 Gbps
6. **Termination**: On-die termination (ODT) enabled for PCIe

## BOM Highlights

| Component | Part | Qty | Supplier |
|-----------|------|-----|----------|
| ThunderX3 | Marvell CN9180-CBO | 1 | DigiKey |
| EPYC | AMD EPYC 7002 Series | 1 | Arrow |
| DDR5 | Samsung M471A4K43CB1-CTD | 8 | Mouser |
| PCIe Switch | N/A (integrated in ThunderX3) | 1 | - |
| BMC | ASPEED AST2600 | 1 | Avnet |
| TPM | Infineon I2F75 | 1 | DigiKey |
| NVMe | Samsung PM9A3 2TB | 1 | Mouser |
| Power IC | TI TPS546D2A | 4 | DigiKey |
| Thermal | TI TMP468 | 1 | Avnet |
