# Board Trace: MTS-RG-500 Residential Gateway

## Overview

MTS-RG-500 Residential Gateway for FTTH B2C deployment (GPON ONU + WiFi 6 + VoIP)
on MediaTek MT7981 (Filco) + Realtek RTL960x GPON PHY architecture.

## Hardware Architecture

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                       MTS-RG-500 MAIN BOARD                                 │
│                                                                              │
│  ┌──────────────────────────┐  ┌──────────────────────────┐                 │
│  │  MediaTek MT7981         │  │  Realtek RTL960x         │                 │
│  │  (Filco)                 │  │  GPON PHY Controller     │                 │
│  │                          │  │                          │                 │
│  │  Dual-core Cortex-A53    │  │  ┌────────────────────┐  │                 │
│  │  @ 2.0 GHz               │  │  │ GPON MAC Engine    │  │                 │
│  │                          │  │  │ OMCI Management    │  │                 │
│  │  DDR3-1600               │  │  │ 128x ONU mgmt      │  │                 │
│  │  512 MB Samsung          │  │  │ TR-069 Agent       │  │                 │
│  │  M471A4 K3F6             │  │  └────────────────────┘  │                 │
│  │                        │  │         │ I2C/SPI         │                 │
│  │  WiFi 6 (MT76)         │  │  ┌─────▼────────┐        │                 │
│  │  integrated PCB        │  │  │ GPON PHY     │        │                 │
│  │  antennas (2.4+5 GHz)  │  │  │ (128x ONU)   │        │                 │
│  │                        │  │  └──────────────┘        │                 │
│  │  USB 2.0 / 3.0         │  │                          │                 │
│  │  SDMMC (eMMC)          │  │  ┌────────────────────┐  │                 │
│  │  256 MB NAND           │  │  │ 4x GE PHY          │  │                 │
│  │  Macronix MX25L ...    │  │  │ (RTL8211F)         │  │                 │
│  └────────────────────────┘  │  └────────────────────┘  │                 │
│                               └──────────────────────────┘                 │
│                                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐              │
│  │  EEPROM      │  │  Crystal     │  │  Power               │              │
│  │  24AA02E64   │  │  40MHz       │  │  LDO 3.3V/1.0V     │              │
│  │  I2C addr    │  │  TCXO        │  │  12V DC jack       │              │
│  └──────────────┘  └──────────────┘  └──────────────────────┘              │
│                                                                              │
│  ┌────────────────────────────────────────────────────────────────────────┐  │
│  │                    Port Panel                                          │  │
│  │                                                                        │  │
│  │  GPON (SC/APC)  ETH1  ETH2  ETH3  ETH4  USB  POTS  HDMI             │  │
│  │       │          │     │     │     │            │     │               │  │
│  │       └──────────┘     └─────┴─────┘            └─────┘               │  │
│  └────────────────────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────────────────┘
```

## I2C Tracing

### MT7981 ↔ RTL960x (I2C GPON Management)

| Signal | MT7981 Pin | RTL960x Pin | Pull-up |
|--------|------------|-------------|---------|
| SDA | PA4 (I2C0) | PIN 45 | 4.7kΩ |
| SCL | PA5 (I2C0) | PIN 44 | 4.7kΩ |

### MT7981 ↔ EEPROM (I2C)

| Signal | MT7981 Pin | EEPROM Pin | Pull-up |
|--------|------------|------------|---------|
| SDA | PB0 (I2C1) | PIN 5 | 4.7kΩ |
| SCL | PB1 (I2C1) | PIN 4 | 4.7kΩ |

### MT7981 ↔ Thermal (I2C)

| Signal | MT7981 Pin | TMP Pin | Pull-up |
|--------|------------|---------|---------|
| SDA | PC0 (I2C2) | PIN 15 | 4.7kΩ |
| SCL | PC1 (I2C2) | PIN 14 | 4.7kΩ |

## SPI Tracing

### MT7981 ↔ SPI Flash

| Signal | MT7981 Pin | Flash Pin | Length |
|--------|------------|-----------|--------|
| MOSI | PD0 (SPI0) | PIN 5 | < 5 mm |
| MISO | PD1 (SPI0) | PIN 2 | < 5 mm |
| SCK | PD2 (SPI0) | PIN 6 | < 5 mm |
| CS | PD3 (SPI0) | PIN 4 | < 5 mm |

## PCIe Tracing

### MT7981 ↔ WiFi 6 (MT76)

| Lane | MT7981 Pin | MT76 Pin | Impedance | Length |
|------|------------|----------|-----------|--------|
| PCIe Lane 0 | PE0 | PIN 1 | 50Ω | < 10 mm |
| Lane 1 | PE1 | PIN 2 | 50Ω | < 10 mm |
| Lane 2 | PE2 | PIN 3 | 50Ω | < 10 mm |
| Lane 3 | PE3 | PIN 4 | 50Ω | < 10 mm |

## DDR3 Tracing

### Memory bus

| Signal | Pin | Impedance | Length |
|--------|-----|-----------|--------|
| DQ0-DQ31 | A1-A32 | 40Ω | < 3 mm |
| Address | B1-B8 | 50Ω | < 3 mm |
| CLK | C1 | 40Ω diff | < 3 mm |
| CS | D1 | 50Ω | < 3 mm |
| VDD | - | 1.5V | - |

## Ethernet PHY Tracing (RTL8211F)

### MT7981 RGMII ↔ RTL8211F

| Signal | MT7981 Pin | PHY Pin | Length |
|--------|------------|---------|--------|
| RGMII-TXCLK | PA10 | CLK+ | < 5 mm |
| RGMII-RXCLK | PA11 | CLK- | < 5 mm |
| TXD0 | PA12 | D0 | < 5 mm |
| TXD1 | PA13 | D1 | < 5 mm |
| TXD2 | PA14 | D2 | < 5 mm |
| TXD3 | PA15 | D3 | < 5 mm |
| TXEN | PA16 | EN+ | < 5 mm |
| RXD0 | PA17 | D4 | < 5 mm |
| RXD1 | PA18 | D5 | < 5 mm |
| RXD2 | PA19 | D6 | < 5 mm |
| RXD3 | PA20 | D7 | < 5 mm |
| RXD1 | PA21 | EN- | < 5 mm |
| MDIO | PA22 | MDIO | < 5 mm |
| MDCK | PA23 | MDC | < 5 mm |

## GPIO Mapping

| GPIO | Function | State |
|------|----------|-------|
| GPIO55 | PWR LED | Output (active low) |
| GPIO56 | WAN LED | Output (active low) |
| GPIO57 | WLAN2G LED | Output (active low) |
| GPIO58 | WLAN5G LED | Output (active low) |
| GPIO59 | GPON LED | Output (active low) |
| GPIO60 | FAULT LED | Output (active low) |
| GPIO61 | RESET BTN | Input (pull-up) |
| GPIO62 | WPS BTN | Input (pull-up) |
| GPIO30 | VCC_GPON Reg | Output (regulator enable) |
| GPIO31 | VDDA25_WIFI | Output (regulator enable) |
| GPIO32 | VDDA30_WIFI | Output (regulator enable) |
| GPIO33 | GPON Reset | Output (active high) |

## PCB Parameters

| Parameter | Value |
|-----------|-------|
| Layers | 4-layer |
| Material | FR-4 |
| Impedance | 50Ω single |
| Copper | 1 oz (outer) |
| Surface | ENIG |
| Thickness | 1.6 mm |
| Dimensions | 150 x 100 mm |

## Power Distribution

### Voltage Rails

| Rail | Voltage | Current | Regulator |
|------|---------|---------|-----------|
| V_CORE | 1.0V | 3A | MP2483 |
| V_DDR3 | 1.5V | 1A | MP1584 |
| V_IO | 3.3V | 500mA | RT9013 |
| V_GPON | 3.3V | 300mA | LDO |
| V_WIFI | 2.5V/3.3V | 500mA | LDO x2 |
| V_AUX | 3.3V | 1A | LDO |

### Power consumption

| Mode | Power |
|------|-------|
| Idle | 6W |
| Load | 11W |
| Peak | 12W |

## Enclosure

| Parameter | Value |
|-----------|-------|
| Dimensions | 150 x 100 x 30 mm |
| Weight | 200 g (max) |
| Material | ABS plastic |
| Mounting | Wall-mount / desktop |
| Ventilation | Passive convection |
| IP Rating | IP20 |
