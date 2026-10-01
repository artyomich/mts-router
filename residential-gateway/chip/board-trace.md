# Board Trace: MTS-RG-500 Residential Gateway

## Overview

MTS-RG-500 Residential Gateway для домашних и малых офисных подключений GPON
на базе MediaTek MT7981 (Helio X30) + Realtek RTL960x GPON PHY.

## Hardware Architecture

```
┌────────────────────────────────────────────────────────────────────────────┐
│                       MTS-RG-500 MAIN BOARD                               │
│                                                                            │
│  ┌──────────────────────┐  ┌──────────────────────────┐                   │
│  │  MediaTek MT7981     │  │  Realtek RTL960x         │                   │
│  │  (Cortex-A53 x2)     │  │  GPON PHY Controller     │                   │
│  │  @ 2.0GHz            │  │                          │                   │
│  │                      │  │  ┌────────────────────┐  │                   │
│  │  DDR4-3200           │  │  │ GPON TX 1490nm     │  │                   │
│  │  512MB ECC           │  │  │ GPON RX 1310nm     │  │                   │
│  │  @ 3200MT/s          │  │  │ OMCI management    │  │                   │
│  │                      │  │  │ 128x ONU mgmt      │  │                   │
│  │  WiFi 6 (MT76)       │  │  │ TR-069 agent       │  │                   │
│  │  2.4GHz + 5GHz       │  │  └────────────────────┘  │                   │
│  │  802.11ax            │  │                          │                   │
│  │  2x2 MIMO            │  │  I2C connection          │                   │
│  │  4x4 MIMO (5GHz)     │  └──────────────────────────┘                   │
│  │                      │         │                                        │
│  │  Ethernet MAC x2     │         │ I2C bus                                  │
│  │  USB 3.0 / 2.0       │         │                                        │
│  │  PCIe 2.0            │         └───────────────────────────────────────┐  │
│  │  SDMMC               │                                                 │  │
│  └──────────┬───────────┘                                                 │  │
│             │  I2C / SPI / GPIO                                           │  │
│             │                                                               │  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐            │  │
│  │  EEPROM      │  │  Crystal     │  │  Power               │            │  │
│  │  24AA02E64   │  │  40MHz       │  │  AP6256 (WiFi)      │            │  │
│  │  I2C addr    │  │  TCXO        │  │  3.3V regulator     │            │  │
│  └──────────────┘  └──────────────┘  └──────────────────────┘            │  │
│                                                                            │  │
│  ┌────────────────────────────────────────────────────────────────────┐    │  │
│  │                    Port Panel                                      │    │  │
│  │                                                                    │    │  │
│  │  PON (SC/APC)  RJ45-1  RJ45-2  RJ45-3  RJ45-4                    │    │  │
│  │    │              │       │       │       │                       │    │  │
│  │    └──────────────┘       │       │       │                       │    │  │
│  │                           └───────┴───────┘                       │    │  │
│  │                                    │                               │    │  │
│  │                            MT7981 GMAC                            │    │  │
│  └────────────────────────────────────────────────────────────────────┘    │
│                                                                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐            │
│  │  LED array   │  │  Buttons     │  │  USB ports           │            │
│  │  PWR/WAN/    │  │  Reset/WPS   │  │  USB3.0 + USB2.0     │            │
│  │  WLAN/GPON   │  │  (GPIO)      │  │  (for storage)       │            │
│  └──────────────┘  └──────────────┘  └──────────────────────┘            │
└────────────────────────────────────────────────────────────────────────────┘
```

## I2C Tracing

### GPON PHY Management Bus

```
MT7981 I2C0 (AP SoC)
│
├── SDA0 → RTL960x GPON PHY (addr 0x40)
├── SCL0 → RTL960x
├── SDA0 → EEPROM 24AA02E64 (addr 0x50)
├── SCL0 → EEPROM
├── SDA0 → WiFi AP6256 (I2C control, addr 0x08)
├── SCL0 → WiFi
└── SDA0 → Thermal sensor (internal)
    └── SCL0 → Thermal
```

## SPI Flash Tracing

### Boot Flash

```
MT7981 SPI0
│
├── SPI0_MOSI → SPI Flash (Winbond W25Q64)
├── SPI0_MISO → SPI Flash
├── SPI0_SCLK → SPI Flash
├── SPI0_CS → SPI Flash (GPIO)
├── Speed: up to 104 MHz (SPI Quad Mode)
├── Interface: SPI 3.3V
└── Flash: 8MB (firmware + config)
```

## DDR4 Tracing

### Memory Layout

```
MT7981 Memory Controller
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

## WiFi RF Layout

### MT76 WiFi 6 Front-End

```
MT7981 WiFi MAC (integrated)
│
├── 2.4GHz Path:
│   ├── TX: MT7981 → PA (Power Amp) → Filter → Antenna 1/2
│   ├── RX: Antenna 1/2 → LNA (Low Noise Amp) → Filter → MT7981
│   ├── Frequency: 2.402-2.480 GHz
│   ├── Power: +20dBm TX, -95dBm RX sensitivity
│   └── MIMO: 2x2
│
└── 5GHz Path:
    ├── TX: MT7981 → PA → Filter → Antenna 1-4
    ├── RX: Antenna 1-4 → LNA → Filter → MT7981
    ├── Frequency: 5.150-5.825 GHz
    ├── Power: +24dBm TX, -90dBm RX sensitivity
    └── MIMO: 4x4
```

## Power Sequence

```
1. 5V EXT (from external PSU)
   │
   ├── Powers 5V rail
   └── Powers RJ45 magnetics
   │
2. 3.3V MAIN (regulator, enabled by BMC)
   │
   ├── Powers MT7981 I/O
   ├── Powers RTL960x
   ├── Powers WiFi RF
   └── Powers flash
   │
3. 1.2V DDR (regulator, enabled by PMIC)
   │
   └── Powers DDR4 VDD/VDDQ
   │
4. 1.0V CORE (regulator, enabled by PMIC)
   │
   └── Powers MT7981 core
   │
5. PON laser enable (GPIO controlled)
   │
   └── Powers GPON TX laser
```

## Signal Integrity Notes

1. **SPI Flash**: Length-matched within 5 mils, controlled impedance
2. **DDR4**: All DQ lines matched within 15 mils, 40Ω single-ended
3. **Reference clocks**: 40MHz TCXO for MT7981
4. **RF paths**: 50Ω controlled impedance for WiFi antennas
5. **Power planes**: 4-layer minimum, split analog/digital/RF
6. **Optical path**: Controlled impedance for laser driver
7. **GPON**: Class B+ optical (20dB reach, 28dB sensitivity)

## Clock Distribution

```
40MHz TCXO (MT7981 main clock)
│
├── Output 1 → MT7981 main clock
└── Output 2 → Reserved
```

## Optical Specifications

| Parameter | Value | Description |
|-----------|-------|-------------|
| TX Wavelength | 1490nm | GPON data upstream |
| RX Wavelength | 1310nm | GPON data downstream |
| RF Wavelength | 1550nm | CATV overlay |
| TX Power | 0 to +5 dBm | Class B+ |
| RX Sensitivity | -28 dBm | Class B+ |
| Split Ratio | 1:128 | Standard GPON |
| Reach | 20km | Class B+ |
| Connector | SC/APC | Physical contact |

## BOM Highlights

| Component | Part | Qty | Supplier |
|-----------|------|-----|----------|
| MT7981 | MediaTek MT7981A | 1 | DigiKey |
| RTL960x | Realtek RTL960x | 1 | Avnet |
| DDR4 | Micron MT52L256M32D1DI | 1 | Mouser |
| EEPROM | Microchip 24AA02E64 | 1 | DigiKey |
| WiFi | Broadcom AP6256 | 1 | DigiKey |
| Flash | Winbond W25Q64 | 1 | Mouser |
| Regulator | TI TPS62130 | 3 | DigiKey |
| TCXO | 40MHz TCXO | 1 | DigiKey |
| GPON TX | Lumentum L1540S07A | 1 | Avnet |
| GPON RX | Lumentum R1540S07A | 1 | Avnet |

## Port Mapping

| Port | Type | Speed | Description |
|------|------|-------|-------------|
| PON | SC/APC | 2.5G/1.25G | GPON optical |
| RJ45-1 | RJ45 | 1G | WAN (optional) |
| RJ45-2-4 | RJ45 | 1G | LAN ports |
| USB3.0 | USB3.0 | 5Gbps | Storage |
| USB2.0 | USB2.0 | 480M | Storage/3G/4G |
| MGMT | RJ45 | 1G | Management |
| CONSOLE | UART | 115200 | Debug |
