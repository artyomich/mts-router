# MTS-RG-500 — Residential Gateway

## Спецификация домашнего шлюза (FTTH B2C)

---

## 1. Назначение

MTS-RG-500 — домашний шлюз для FTTH-сегмента МТС, обеспечивающий:
- GPON ONU/ONT для подключения к сети МТС
- WiFi 6 (802.11ax) для беспроводного доступа
- VoIP для телефонии
- IPTV для телевидения
- TR-069 для удаленного управления
- Home networking (LAN, DHCP, NAT, firewall)

---

## 2. Аппарная спецификация

### 2.1 Основная плата (Main Board)

| Компонент | Спецификация | Примечание |
|-----------|-------------|------------|
| **SoC** | MediaTek MT7981 (Filco) | ARM Cortex-A53 quad-core @ 2 GHz |
| **WiFi** | WiFi 6 (802.11ax) integrated | 2x2 MIMO, 2.4 GHz + 5 GHz |
| **GPON PHY** | Realtek RTL960x | GPON PHY + MAC |
| **Memory** | 512 MB DDR3 | |
| **Storage** | 256 MB NAND Flash | Boot + OS |
| **Crypto** | Hardware AES, RSA | |
| **Power** | 12W DC | 12V/1A |
| **Dimensions** | Compact | 150 x 100 x 30 mm |

### 2.2 Порты

| Тип | Кол-во | Спецификация |
|-----|--------|-------------|
| **GPON** | 1x | SC/APC connector |
| **Ethernet** | 4x | GE (10/100/1000) |
| **USB** | 1x | USB 2.0 |
| **POTS** | 1x | RJ11 (VoIP) |
| **Coax** | 1x | IPTV (optional) |
| **HDMI** | 1x | IPTV (optional) |

### 2.3 Антенны

| Параметр | Значение |
|----------|----------|
| **WiFi** | Internal PCB antennas |
| **2.4 GHz** | 2x2 MIMO, 4 dBi |
| **5 GHz** | 2x2 MIMO, 4 dBi |
| **Bluetooth** | 5.0 (optional) |

---

## 3. Сетевые возможности

### 3.1 GPON спецификация

| Параметр | Значение |
|----------|----------|
| **Standard** | ITU-T G.984 (GPON) |
| **Downstream** | 2.488 Gbps |
| **Upstream** | 1.244 Gbps |
| **Distance** | до 60 km |
| **Split Ratio** | 1:128 |

### 3.2 Протоколы

| Протокол | Версия | Поддержка |
|----------|--------|-----------|
| GPON | G.984 | ✓ |
| OMCI | ME-198 | ✓ |
| TR-069 | CWMP | ✓ |
| DHCP | RFC 2131 | ✓ |
| PPPoE | RFC 2516 | ✓ |
| VLAN | 802.1Q | ✓ |
| DNS | — | ✓ |
| NAT | — | ✓ |
| Firewall | Stateful | ✓ |
| WiFi | 802.11ax | ✓ |
| WPA3 | — | ✓ |

### 3.3 VoIP

| Параметр | Значение |
|----------|----------|
| **Codec** | G.711, G.729, Opus |
| **Protocol** | SIP |
| **Ports** | 1x POTS |
| **QoS** | RTP priority |

### 3.4 IPTV

| Параметр | Значение |
|----------|----------|
| **Multicast** | IGMP Proxy/Snooping |
| **Protocols** | HLS, RTSP, HTTP |
| **Output** | HDMI, Coax |
| **Decoding** | H.264, H.265 |

### 3.5 QoS

| Параметр | Значение |
|----------|----------|
| **Classes** | 8 per port |
| **Scheduling** | WRR, Strict |
| **Marking** | DSCP, PCP |

---

## 4. Программная платформа

### 4.1 Операционная система

| Параметр | Значение |
|----------|----------|
| **Base OS** | OpenWrt (Linux) |
| **Kernel** | 6.6 LTS (custom) |
| **WiFi** | MediaTek MT76 driver |
| **GPON** | RTL960x driver |
| **VoIP** | Asterisk / FreeSWITCH |
| **TR-069** | cwmpd |
| **Web UI** | LuCI |
| **CLI** | OpenWrt CLI |

### 4.2 YANG-модели

```
mts-residential-gateway/
├── mts-rg-gpon.yang
├── mts-rg-wifi.yang
├── mts-rg-voip.yang
├── mts-rg-tr069.yang
├── mts-rg-qos.yang
└── mts-rg-telemetry.yang
```

---

## 5. Требования к Linux-драйверам

### 5.1 MediaTek MT7981

| Драйвер | Назначение |
|---------|------------|
| `mt7981-net` | Ethernet TX/RX |
| `mt7981-wifi` | WiFi 6 (mac80211) |
| `mt7981-crypto` | Hardware AES/RSA |

### 5.2 Realtek RTL960x

| Драйвер | Назначение |
|---------|------------|
| `rtl960x-gpon` | GPON PHY + MAC |
| `rtl960x-omci` | OMCI management |

---

## 6. Энергопотребление

| Режим | Потребление |
|-------|-------------|
| **Idle** | 6W |
| **Load** | 11W |
| **Peak** | 12W |

---

## 7. Физические параметры

| Параметр | Значение |
|----------|----------|
| **Размеры** | 150 x 100 x 30 mm |
| **Вес** | 200 g (max) |
| **Рабочая темп.** | 0°C ... +45°C |
| **Влажность** | 10% ... 90% non-condensing |
| **Шум** | 0 dB(A) (passive) |
| **Protection** | IP20 |

---

## 8. API спецификация (REST)

### 8.1 Основные эндпоинты

```
GET    /api/v1/gpon               — GPON state
GET    /api/v1/wifi               — WiFi state
PUT    /api/v1/wifi/<band>        — WiFi config
GET    /api/v1/voip               — VoIP state
PUT    /api/v1/voip               — VoIP config
GET    /api/v1/tr069             — TR-069 config
GET    /api/v1/iptv              — IPTV state
GET    /api/v1/lan               — LAN config
GET    /api/v1/telemetry         — Telemetry
GET    /api/v1/health            — Health check
```

### 8.2 Пример запроса (WiFi config)

```json
{
  "method": "PUT",
  "url": "/api/v1/wifi/5ghz",
  "body": {
    "ssid": "MTS_Home_5G",
    "channel": 36,
    "bandwidth": "80MHz",
    "security": "wpa3",
    "password": "***",
    "guest": false
  }
}
```

---

## 9. Трассировка платы

### 9.1 Основные сигналы

| Сигнал | Тип | Скорость | Примечание |
|--------|-----|----------|------------|
| PCIe Gen2 x1 | SoC ↔ WiFi | 5 GT/s | WiFi 6 |
| DDR3 | SoC ↔ DDR3 | 1600 MT/s | 512 MB |
| I2C | SoC ↔ RTL960x | 100 kHz | GPON mgmt |
| SPI | SoC ↔ Flash | 50 MHz | Boot |
| GPON | SoC ↔ RTL960x | 2.5 Gbps | GPON |
| POTS | SoC ↔ Codec | — | VoIP |

### 9.2 PCB параметры

| Параметр | Значение |
|----------|----------|
| **Layers** | 4-layer |
| **Material** | FR-4 |
| **Impedance** | 50Ω single |
| **Copper** | 1 oz (outer) |
| **Surface** | ENIG |

---

## 10. Checklist для разработки

- [ ] Проектирование main board (MT7981 + RTL960x)
- [ ] Разработка драйверов MT7981
- [ ] Разработка драйверов RTL960x
- [ ] Сборка OpenWrt образа
- [ ] Интеграция WiFi 6 (MT76)
- [ ] Интеграция TR-069
- [ ] Разработка VoIP (Asterisk)
- [ ] Разработка IPTV
- [ ] Разработка YANG моделей
- [ ] Разработка REST API
- [ ] Тестирование GPON
- [ ] Тестирование WiFi 6
- [ ] Тестирование VoIP
- [ ] Тестирование IPTV
- [ ] Сертификация
