# Board Trace: MTS-OLT-2000 GPON OLT

## Overview

MTS-OLT-2000 GPON OLT для массового развертывания GPON инфраструктуры МТС
на базе Intel Tofino 2 + AMD EPYC + Realtek RTL960x.

## Hardware Architecture

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                       MTS-OLT-2000 MAIN BOARD                               │
│                                                                              │
│  ┌──────────────┐    ┌──────────────────────────────┐    ┌──────────────┐  │
│  │  AMD EPYC    │    │    Intel Tofino 2 BFN128     │    │   DDR5       │  │
│  │  7002 Series │    │    (Silicon Packet Processor)│    │  4x 32GB     │  │
│  │  32-Core     │    │    16nm PCIe 3.0             │    │  5120MT/s    │  │
│  │  @ 2.6GHz    │    │                              │    │  (1TB total)   │  │
│  │              │    │   ┌────────────────────────┐ │    │              │  │
│  │  PCIe Gen4   │───►│   │ 192x 10G/25G MAC Ports │ │    │  DDR5-5120   │  │
│  │  x64 total   │    │   │ 64x 100G MAC Ports     │ │    │  Samsung     │  │
│  │              │    │   │ P4 Programmable Pipeline│ │    │  M471A4     │  │
│  │  DDR5 ECC    │    │   └────────────────────────┘ │    │  K3F6         │  │
│  │  4x channels │    │         │                    │    │                │  │
│  └──────────────┘    │    ┌────────────────────────┐ │    └──────────────┘  │
│                      │    │  AMD FP5 BMC           │ │                     │
│                      │    │  (AST2600)             │ │    ┌──────────────┐ │
│                      │    │  IPMI 2.0              │ │    │  NVMe SSD    │ │
│                      │    └────────────────────────┘ │    │  1TB         │ │
│                      │         │                      │    │  Samsung     │ │
│                      │    ┌────────────────────────┐ │    │  PM9A3        │ │
│                      │    │  Realtek RTL960x       │ │    │  ZT1TL        │ │
│                      │    │  GPON PHY Controller   │ │    └──────────────┘ │
│                      │    │  OMCI Management       │ │                     │
│                      │    │  TR-069 Agent          │ │    ┌──────────────┐ │
│                      │    │  128x ONU management   │ │    │  PSU         │ │
│                      │    └────────────────────────┘ │    │  350W        │ │
│                      │                                │    └──────────────┘ │
│                      └────────────────────────────────┘                     │
│                                                                              │
│  ┌────────────────────────────────────────────────────────────────────────┐  │
│  │                    32x GPON Ports (1:128 split)                        │  │
│  │                                                                        │  │
│  │  PON0    PON1    PON2    PON3    PON4    PON5    PON6    PON7        │  │
│  │  PON8    PON9    PON10   PON11   PON12   PON13   PON14   PON15       │  │
│  │  PON16   PON17   PON18   PON19   PON20   PON21   PON22   PON23       │  │
│  │  PON24   PON25   PON26   PON27   PON28   PON29   PON30   PON31       │  │
│  │    │       │       │       │       │       │       │       │          │  │
│  │    └───────┴───────┴───────┴───────┴───────┴───────┴───────┘          │  │
│  │                    │                                                    │  │
│  │            RTL960x GPON PHY Controller                                 │  │
│  └────────────────────────────────────────────────────────────────────────┘  │
│                                                                              │
│  ┌────────────────────────────────────────────────────────────────────────┐  │
│  │                    4x 100G Uplink Ports                                │  │
│  │                                                                        │  │
│  │  QSFP0   QSFP1   QSFP2   QSFP3                                        │  │
│  │    │       │       │       │                                           │  │
│  │    └───────┴───────┴───────┘                                           │  │
│  │                    │                                                    │  │
│  │            Tofino 2 MAC Ports                                          │  │
│  └────────────────────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────────────────┘
```

## PCIe Tracing

### CPU ↔ Tofino 2 Connection

```
AMD EPYC 7002 (PCIe Root Complex)
│
├── Lane 0-15  →  Tofino 2 BFN128 Slot 0 (x16)
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
├── Lane 16-31 →  Tofino 2 BFN128 Slot 0 (x16, continued)
│   ├── Signal: PCIe_P0B_TX_P[0:15]
│   ├── Signal: PCIe_P0B_TX_N[0:15]
│   ├── Signal: PCIe_P0B_RX_P[0:15]
│   ├── Signal: PCIe_P0B_RX_N[0:15]
│   ├── Impedance: 100Ω differential
│   └── Length: < 1 inch
│
├── Lane 32-47 →  NVMe SSD (M.2 Key M)
│   ├── Signal: PCIe_M2_TX_P[0:15]
│   ├── Signal: PCIe_M2_TX_N[0:15]
│   ├── Signal: PCIe_M2_RX_P[0:15]
│   ├── Signal: PCIe_M2_RX_N[0:15]
│   ├── Impedance: 100Ω differential
│   └── Length: < 2 inch (M.2 form factor)
│
└── Lane 48-63 →  BMC (PCIe Gen3 x1)
    ├── Signal: PCIe_BMC_TX_P/N
    ├── Signal: PCIe_BMC_RX_P/N
    ├── Impedance: 100Ω differential
    └── Length: < 2 inch
```

## I2C Tracing

### GPON PHY Management Bus

```
Tofino 2 GPIO (via I2C mux)
│
├── SDA → RTL960x GPON PHY (addr 0x40/0x41)
├── SCL → RTL960x
├── SDA → EEPROM 24AA02E64 (addr 0x50)
├── SCL → EEPROM
├── SDA → Thermal Sensor TI TMP468 (addr 0x4C)
├── SCL → Thermal
├── SDA → BMC (AST2600, addr 0x30)
├── SCL → BMC
├── SDA → PSU Controller (addr 0x34)
└── SCL → PSU
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
└── Channel 3 (DIMM3)
    ├── DQ[0:63]     →  U11 (Samsung M471A4)
    └── Timing: same as Ch0
```

## Power Sequence

```
1. 3.3V AUX (always on from PSU)
   │
   ├── Powers BMC standby
   ├── Powers EEPROM
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
   └── Powers Tofino 2 core
   │
4. VCC_DDR (1.1V, enabled by BMC)
   │
   └── Powers DDR VDDQ
   │
5. VCC_GPON (3.3V, enabled by PMIC)
   │
   ├── Powers RTL960x core
   ├── Powers PON lasers
   └── Powers RF output
   │
6. PCIe PERST# (deasserted by BMC)
   │
   ├── Tofino 2 initializes
   └── NVMe powers on
```

## GPON Optical Layout

```
RTL960x GPON Controller
│
├── TX1490 (1490nm laser) → splitter → 128x ONU
│   ├── Power: 0 to +5 dBm
│   ├── Wavelength: 1490nm ± 10nm
│   └── Safety: Class 1 laser (IEC 60825)
│
├── RX1310 (1310nm receiver) ← ONU ←
│   ├── Sensitivity: -28 dBm
│   ├── Wavelength: 1310nm ± 60nm
│   └── OMCI management channel
│
├── TX1550 (RF upstream) → splitter → RF upstream
│   ├── Center freq: 1550MHz
│   ├── RFoG channel
│   └── CATV overlay
│
└── RX850 (management)
    ├── OMCI messages
    ├── TR-069 data
    └── SNMP traps
```

## Signal Integrity Notes

1. **PCIe**: All lanes length-matched within 5 mils, 100Ω differential impedance
2. **DDR5**: All DQ lines matched within 20 mils, 40Ω single-ended
3. **Reference clocks**: 100MHz, length-matched within 5 mils
4. **Optical paths**: Controlled impedance for laser drivers
5. **Power planes**: 8-layer minimum, split analog/digital/optical
6. **Via stubs**: Back-drilled for PCIe > 5 Gbps
7. **Termination**: On-die termination (ODT) enabled for PCIe/DDR

## BOM Highlights

| Component | Part | Qty | Supplier |
|-----------|------|-----|----------|
| Tofino 2 | Intel BFN128 | 1 | Intel Direct |
| EPYC | AMD EPYC 7002 Series | 1 | Arrow |
| RTL960x | Realtek RTL960x | 1 | DigiKey |
| DDR5 | Samsung M471A4K43CB1-CTD | 4 | Mouser |
| BMC | ASPEED AST2600 | 1 | Avnet |
| NVMe | Samsung PM9A3 1TB | 1 | Mouser |
| EEPROM | Microchip 24AA02E64 | 1 | DigiKey |
| Thermal | TI TMP468 | 1 | Avnet |
| PSU | Delta Electronics 350W | 1 | Mouser |

## Port Mapping

| Port | Type | Speed | Description |
|------|------|-------|-------------|
| PON0-31 | GPON SC/APC | 2.5G/1.25G | 32 GPON ports (1:128 split) |
| QSFP0-3 | QSFP28 | 100G | 4 uplink ports |
| MGMT | RJ45 | 1G | Management |
| USB | USB3.0 | 5Gbps | Service port |
