# MTS-RG-500 — Трассировка платы (Main Board)

## 1. Архитектура платы

```
┌─────────────────────────────────────────────────────────────────────┐
│                      MTS-RG-500 MAIN BOARD                          │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐    │
│  │ MediaTek │    │ Realtek  │    │ WiFi 6   │    │ WiFi 6   │    │
│  │ MT7981   │    │ RTL960x  │    │ MT7676   │    │ MT7676   │    │
│  │ (SoC)    │    │ (GPON)   │    │ (2.4GHz) │    │ (5GHz)   │    │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘    │
│       │               │               │               │            │
│  ┌────▼───────────────▼───────────────▼───────────────▼─────┐     │
│  │              DDR3L 256 MB + SPI Flash 128 MB             │     │
│  └────┬─────────────────────────────────────────────────────┘     │
│       │                                                            │
│  ┌────▼─────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ PHY      │  │ Audio    │  │ USB      │  │ Power    │        │
│  │ LDO      │  │ Codec    │  │ 2.0      │  │ Mgmt     │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. Ethernet трассировка

### 2.1 MT7981 ↔ RJ45 connectors (4x GE)

| Port | MT7981 Pin | RJ45 Pin | Type | Length |
|------|------------|----------|------|--------|
| LAN 1 | J12A1 | X1-1 | RGMII | < 30 mm |
| LAN 2 | J12A2 | X2-1 | RGMII | < 30 mm |
| LAN 3 | J12A3 | X3-1 | RGMII | < 30 mm |
| LAN 4 | J12A4 | X4-1 | RGMII | < 30 mm |

### 2.2 RGMII differential pair routing

| Signal | MT7981 Pin | RJ45 Pin | Impedance | Length Match |
|--------|------------|----------|-----------|-------------|
| TX_CLK | K14A1 | X1-5 | 50Ω | < 0.5 mm |
| TX_CTL | K14A2 | X1-6 | 50Ω | < 0.5 mm |
| TXD0 | K14A3 | X1-7 | 50Ω | < 0.5 mm |
| TXD1 | K14A4 | X1-8 | 50Ω | < 0.5 mm |
| TXD2 | K14A5 | X1-9 | 50Ω | < 0.5 mm |
| TXD3 | K14A6 | X1-10 | 50Ω | < 0.5 mm |
| RX_CLK | K15A1 | X1-11 | 50Ω | < 0.5 mm |
| RX_CTL | K15A2 | X1-12 | 50Ω | < 0.5 mm |
| RXD0 | K15A3 | X1-13 | 50Ω | < 0.5 mm |
| RXD1 | K15A4 | X1-14 | 50Ω | < 0.5 mm |
| RXD2 | K15A5 | X1-15 | 50Ω | < 0.5 mm |
| RXD3 | K15A6 | X1-16 | 50Ω | < 0.5 mm |

## 3. GPON трассировка

### 3.1 MT7981 ↔ RTL960x (SPI + PCIe)

| Signal | MT7981 Pin | RTL960x Pin | Type | Length |
|--------|------------|-------------|------|--------|
| SPI_CLK | J16A1 | M20 | 1.8V | < 10 mm |
| SPI_MOSI | J16A2 | M21 | 1.8V | < 10 mm |
| SPI_MISO | J16A3 | M22 | 1.8V | < 10 mm |
| SPI_CS | J16A4 | M23 | 1.8V | < 10 mm |
| PCIe_CLK | J17A1 | M24 | LVDS | < 10 mm |
| PCIe_RST | J17A2 | M25 | 1.8V | < 10 mm |

### 3.2 RTL960x ↔ SC/APC connector

| Signal | RTL960x Pin | SC/APC Pin | Type | Length |
|--------|-------------|------------|------|--------|
| PON_TX | N14A1 | SC-3 | Laser 1490nm | < 20 mm |
| PON_RX | N14A2 | SC-4 | PD 1310nm | < 20 mm |
| RFT_OUT | N15A1 | SC-1 | RF 5-2400MHz | < 10 mm |

## 4. WiFi 6 трассировка

### 4.1 MT7981 ↔ MT7676 (PCIe + USB)

| Signal | MT7981 Pin | MT7676 Pin | Type | Length |
|--------|------------|------------|------|--------|
| PCIe_LNK | J18A1 | W1-1 | CLK 50MHz | < 15 mm |
| PCIe_TXP | J18A2 | W1-2 | LVDS | < 15 mm |
| PCIe_TXN | J18A3 | W1-3 | LVDS | < 15 mm |
| PCIe_RXP | J18A4 | W1-4 | LVDS | < 15 mm |
| PCIe_RXN | J18A5 | W1-5 | LVDS | < 15 mm |
| USB_DP | J19A1 | W1-6 | D+ 3.3V | < 10 mm |
| USB_DN | J19A2 | W1-7 | D- 3.3V | < 10 mm |

### 4.2 WiFi antenna traces

| Antenna | Connector | Type | Impedance | Length |
|---------|-----------|------|-----------|--------|
| 2.4GHz 1 | ANT1 | SMA | 50Ω | < 20 mm |
| 2.4GHz 2 | ANT2 | SMA | 50Ω | < 20 mm |
| 5GHz 1 | ANT3 | SMA | 50Ω | < 20 mm |
| 5GHz 2 | ANT4 | SMA | 50Ω | < 20 mm |

## 5. VoIP/Audio трассировка

### 5.1 Audio codec interface

| Signal | MT7981 Pin | Codec Pin | Type | Length |
|--------|------------|-----------|------|--------|
| I2S_MCLK | K20A1 | C1-1 | 1.8V | < 10 mm |
| I2S_FS | K20A2 | C1-2 | 1.8V | < 10 mm |
| I2S_DIN | K20A3 | C1-3 | 1.8V | < 10 mm |
| I2S_DOUT | K20A4 | C1-4 | 1.8V | < 10 mm |
| I2C_SDA | K20A5 | C1-5 | 1.8V | < 10 mm |
| I2C_SCL | K20A6 | C1-6 | 1.8V | < 10 mm |

## 6. I2C трассировка

### 6.1 MT7981 ↔ All devices

| Signal | MT7981 Pin | Target | Pull-up | Resistor |
|--------|------------|--------|---------|----------|
| I2C0_SDA | K21A1 | RTL960x/EEPROM | 2.2kΩ | R1-R2 |
| I2C0_SCL | K21A2 | RTL960x/EEPROM | 2.2kΩ | R1-R2 |
| I2C1_SDA | K21A3 | Codec/PSU | 2.2kΩ | R3 |
| I2C1_SCL | K21A4 | Codec/PSU | 2.2kΩ | R3 |

## 7. DDR3L трассировка

### 7.1 Memory channels

| Channel | DQ Pins | Address Pins | CLK | Length |
|---------|---------|-------------|-----|--------|
| CH0 | A1-A8 | B1-B4 | C1 | < 3 mm |
| CH1 | A9-A16 | B5-B8 | C2 | < 3 mm |

### 7.2 DDR3L timing

| Parameter | Value |
|-----------|-------|
| **Frequency** | 800 MT/s |
| **VDD** | 1.35 V |
| **VDDQ** | 1.35 V |
| **Impedance** | 40Ω single-ended |
| **Length match** | < 0.3 mm |

## 8. Power delivery

### 8.1 Power rails

| Rail | Voltage | Current | Regulator | Location |
|------|---------|---------|-----------|----------|
| VCCINT | 1.0V | 3A | TI TPS546D2A | MT7981 |
| VCCDDRO | 0.9V | 1.5A | TI TPS546D2A | DDR3L |
| VCC AUX | 3.3V | 2A | ON Semi NCP3031 | Edge |
| VBAT | 3.3V | 50mA | MCP1825 | Audio |

### 8.2 Power sequencing

| Step | Rail | Voltage | Ramp Time | Enable |
|------|------|---------|-----------|--------|
| 1 | VCC AUX | 3.3V | 1ms | PWR_GOOD |
| 2 | VBAT | 3.3V | 0.5ms | PWR_GOOD |
| 3 | VCCINT | 1.0V | 2ms | VCC AUX OK |
| 4 | VCCDDRO | 0.9V | 2ms | VCCINT OK |
| 5 | RESET# | — | — | All rails OK |
