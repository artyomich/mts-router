# MTS-RG-500 Residential Gateway — gRPC API Backend

## Описание проекта

Реализация **gRPC backend на C++** для маршрутизатора **MTS-RG-500** (Residential Gateway).
Показывает уровень владения **Linux subsystems**, **C++ API design** и **домашними телеком-протоколами**.

---

## Что реализовано

### 1. Hardware Abstraction Layer (HAL)

Пять независимых HAL модулей, демонстрирующих работу с Linux subsystem:

#### WifiHal (WiFi 6 Management)
- **Чтение из sysfs**: `/sys/class/ieee80211/` — WiFi interface state, signal strength
- **Чтение из /sys/class/thermal/**: monitoring температуры WiFi чипа
- **Парсинг hostapd logs**: мониторинг WiFi clients (20 mock)
- **iw CLI**: управление WiFi channel и tx_power
- **Thread-safe**: mutex для защиты client table
- **Мок-режим**: для тестирования без hardware

#### VoipHal (VoIP/Telephony)
- **Чтение из /proc/net/udp**: мониторинг RTP streams
- **Asterisk AMI**: управление VoIP calls через Asterisk Manager Interface
- **ALSA**: monitoring audio quality (MOS score, jitter)
- **Thread-safe**: mutex для защиты call table
- **Мок-режим**: для тестирования

#### IptvHal (IPTV Management)
- **Чтение из /proc/net/igmp**: мониторинг IGMP multicast groups
- **ip mroute**: управление multicast routing таблицами
- **Парсинг igmpproxy logs**: мониторинг IPTV channels (12 mock)
- **Thread-safe**: mutex для защиты channel table
- **Мок-режим**: для тестирования

#### GponHal (GPON ONU)
- **Чтение из sysfs**: `/sys/class/gpon/` — GPON ONU state
- **rtl_gpon CLI**: управление GPON портами
- **Thread-safe**: mutex для защиты ONU table
- **Мок-режим**: для тестирования

#### Tr069Hal (TR-069/ACS Monitor)
- **Чтение из /proc/net/tcp**: мониторинг ACS TCP connections
- **Парсинг cwmpd daemon logs**: состояние ACS daemon
- **Thread-safe**: mutex для защиты connection table
- **Мок-режим**: для тестирования

### 2. gRPC Service

Реализация **MtsResidentialService** с 15 RPC методами:

| RPC | Назначение | HAL |
|-----|-----------|-----|
| `GetWifiStatus` | Получить статус WiFi | WifiHal |
| `SetWifiConfig` | Установить конфигурацию WiFi | WifiHal |
| `GetWifiClients` | Получить список WiFi clients | WifiHal |
| `SetWifiChannel` | Установить WiFi channel | WifiHal |
| `GetVoipStatus` | Получить статус VoIP | VoipHal |
| `GetVoipCalls` | Получить список VoIP calls | VoipHal |
| `CreateVoipCall` | Создать VoIP call | VoipHal |
| `DeleteVoipCall` | Удалить VoIP call | VoipHal |
| `GetIptvStatus` | Получить статус IPTV | IptvHal |
| `GetIptvChannels` | Получить список IPTV channels | IptvHal |
| `AddIptvChannel` | Добавить IPTV channel | IptvHal |
| `RemoveIptvChannel` | Удалить IPTV channel | IptvHal |
| `GetGponStatus` | Получить статус GPON | GponHal |
| `GetGponOnu` | Получить список GPON ONU | GponHal |
| `SubscribeTelemetry` | Streaming telemetry | Все HAL |

### 3. Linux Integration

- **sysfs**: прямое чтение из `/sys/class/ieee80211/`, `/sys/class/thermal/`, `/sys/class/power/`, `/sys/class/gpon/`
- **procfs**: парсинг `/proc/net/dev`, `/proc/net/tcp`, `/proc/net/igmp`
- **hostapd CLI**: управление WiFi AP
- **iw CLI**: управление WiFi channel и tx_power
- **Asterisk AMI**: управление VoIP calls
- **ALSA**: monitoring audio quality
- **iproute2**: multicast routing для IPTV
- **rtl_gpon CLI**: GPON port management
- **cwmpd monitoring**: TR-069 ACS connections
- **signal handling**: graceful shutdown при SIGINT/SIGTERM

### 4. C++ Design Patterns

- **RAII**: управление ресурсами через destructors
- **Thread safety**: mutex для защиты общих данных
- **Interface segregation**: HAL как interface
- **Mock mode**: для тестирования без hardware
- **Smart pointers**: unique_ptr, shared_ptr

---

## Структура проекта

```
residential-gateway-api/
├── proto/
│   └── mts_residential.proto          — gRPC protobuf specification
├── include/
│   ├── hal/
│   │   ├── wifi_hal.h                 — WiFi HAL interface
│   │   ├── voip_hal.h                 — VoIP HAL interface
│   │   ├── iptv_hal.h                 — IPTV HAL interface
│   │   ├── gpon_hal.h                 — GPON HAL interface
│   │   └── tr069_hal.h                — TR-069 HAL interface
│   └── service/
│       └── residential_service.h      — gRPC service interface
├── src/
│   ├── hal/
│   │   ├── wifi_hal.cpp               — WiFi HAL implementation
│   │   ├── voip_hal.cpp               — VoIP HAL implementation
│   │   ├── iptv_hal.cpp               — IPTV HAL implementation
│   │   ├── gpon_hal.cpp               — GPON HAL implementation
│   │   └── tr069_hal.cpp              — TR-069 HAL implementation
│   ├── service/
│   │   └── residential_service.cpp    — gRPC service implementation
│   └── main.cpp                       — Server entry point
├── diagrams/
│   ├── C1_overall_system_architecture.puml
│   ├── C2_grpc_service_rpc_methods.puml
│   ├── C3_hal_layer_architecture.puml
│   └── C4_linux_integration_data_flow.puml
├── tests/
│   ├── test_wifi_hal.cpp              — WiFi HAL tests
│   ├── test_voip_hal.cpp              — VoIP HAL tests
│   ├── test_iptv_hal.cpp              — IPTV HAL tests
│   ├── test_gpon_hal.cpp              — GPON HAL tests
│   ├── test_tr069_hal.cpp             — TR-069 HAL tests
│   └── CMakeLists.txt
├── config/
│   └── mts-rg500.conf                 — Server configuration
├── CMakeLists.txt                     — Build configuration
├── Dockerfile                         — Container build
└── README.md                          — Этот файл
```

---

## Сборка и запуск

### Зависимости
```bash
# Ubuntu/Debian
sudo apt-get install -y cmake protobuf-compiler libgrpc++-dev libprotobuf-dev

# Или через vcpkg
vcpkg install protobuf grpc
```

### Сборка
```bash
cd residential-gateway-api
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Запуск
```bash
./mts-residential-gateway-server 0.0.0.0:50054
```

### Тестирование через gRPC CLI
```bash
# Get WiFi status
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetWifiStatus

# Get WiFi clients
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetWifiClients

# Get VoIP status
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetVoipStatus

# Get VoIP calls
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetVoipCalls

# Get IPTV status
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetIptvStatus

# Get IPTV channels
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetIptvChannels

# Get GPON status
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetGponStatus

# Get TR-069 status
grpcurl -plaintext localhost:50054 mts.residential.v1.MtsResidentialService/GetTr069Status

# Subscribe to telemetry
grpcurl -plaintext -d '{"paths":["cpu","memory","temperature"]}' \
    localhost:50054 mts.residential.v1.MtsResidentialService/SubscribeTelemetry
```

---

## Примеры использования

### Python клиент
```python
import grpc
import mts_residential_pb2
import mts_residential_pb2_grpc

channel = grpc.insecure_channel('localhost:50054')
stub = mts_residential_pb2_grpc.MtsResidentialServiceStub(channel)

# Get WiFi status
response = stub.GetWifiStatus(mts_residential_pb2.Empty())
print(f"WiFi Interface: {response.wifi_status.interface_name}")
print(f"SSID: {response.wifi_status.ssid}")
print(f"Channel: {response.wifi_status.channel}")
print(f"Frequency: {response.wifi_status.frequency} MHz")
print(f"TX Power: {response.wifi_status.tx_power} dBm")
print(f"Signal avg: {response.wifi_status.signal_avg} dBm")
print(f"Client count: {response.wifi_status.client_count}")

# Get WiFi clients
response = stub.GetWifiClients(mts_residential_pb2.Empty())
for client in response.clients:
    print(f"MAC: {client.mac_address}, Signal: {client.signal_strength} dBm")
    print(f"  RX: {client.rx_bytes} bytes, TX: {client.tx_bytes} bytes")
    print(f"  Signal avg: {client.signal_avg} dBm, Uptime: {client.uptime} s")

# Create VoIP call
response = stub.CreateVoipCall(
    mts_residential_pb2.CreateVoipCallRequest(
        caller="1001",
        callee="2001",
        codec="G.711"
    )
)
print(f"Call created: {response.call_id}, Duration: {response.duration} s")

# Get IPTV channels
response = stub.GetIptvChannels(mts_residential_pb2.Empty())
for ch in response.channels:
    print(f"Channel {ch.channel_id}: {ch.name} -> {ch.multicast_address}:{ch.multicast_port}")

# Subscribe to telemetry
responses = stub.SubscribeTelemetry(
    mts_residential_pb2.TelemetrySubscription(
        paths=["cpu", "memory", "temperature"],
        sample_interval=1000
    )
)
for response in responses:
    print(f"Timestamp: {response.timestamp}")
    print(f"Metrics: {response.metrics}")
```

### Go клиент
```go
import (
    "context"
    "fmt"
    "google.golang.org/grpc"
    pb "path/to/mts_residential"
)

func main() {
    conn, _ := grpc.Dial("localhost:50054", grpc.WithInsecure())
    defer conn.Close()
    
    client := pb.NewMtsResidentialServiceClient(conn)
    
    // Get WiFi status
    resp, _ := client.GetWifiStatus(context.Background(), &pb.Empty{})
    fmt.Printf("WiFi Interface: %s\n", resp.WifiStatus.InterfaceName)
    fmt.Printf("SSID: %s\n", resp.WifiStatus.Ssid)
    fmt.Printf("Channel: %d\n", resp.WifiStatus.Channel)
    fmt.Printf("Frequency: %d MHz\n", resp.WifiStatus.Frequency)
    fmt.Printf("TX Power: %d dBm\n", resp.WifiStatus.TxPower)
    fmt.Printf("Signal avg: %d dBm\n", resp.WifiStatus.SignalAvg)
    fmt.Printf("Client count: %d\n", resp.WifiStatus.ClientCount)
    
    // Get WiFi clients
    resp, _ = client.GetWifiClients(context.Background(), &pb.Empty{})
    for _, client := range resp.Clients {
        fmt.Printf("MAC: %s, Signal: %d dBm\n", client.MacAddress, client.SignalStrength)
    }
    
    // Create VoIP call
    resp, _ = client.CreateVoipCall(context.Background(), &pb.CreateVoipCallRequest{
        Caller: "1001",
        Callee: "2001",
        Codec: "G.711",
    })
    fmt.Printf("Call created: %s, Duration: %d s\n", resp.CallId, resp.Duration)
    
    // Get IPTV channels
    resp, _ = client.GetIptvChannels(context.Background(), &pb.Empty{})
    for _, ch := range resp.Channels {
        fmt.Printf("Channel %d: %s -> %s:%d\n", ch.ChannelId, ch.Name, ch.MulticastAddress, ch.MulticastPort)
    }
}
```

---

## Уровень реализации для портфолио

### Что показывает этот код:

1. **Работа с Linux kernel interfaces**
   - sysfs (`/sys/class/ieee80211/`, `/sys/class/thermal/`, `/sys/class/power/`, `/sys/class/gpon/`)
   - procfs (`/proc/net/dev`, `/proc/net/tcp`, `/proc/net/igmp`)
   - hostapd CLI — WiFi AP management
   - iw CLI — WiFi channel/power management
   - Asterisk AMI — VoIP call management
   - ALSA — audio quality monitoring
   - iproute2 — multicast routing for IPTV
   - rtl_gpon CLI — GPON port management
   - cwmpd monitoring — TR-069 ACS connections

2. **C++ на уровне embedded**
   - Thread-safe HAL через mutex
   - RAII для управления ресурсами
   - Smart pointers (unique_ptr, shared_ptr)
   - Mock mode для тестирования
   - Signal handling (SIGINT, SIGTERM)

3. **Домашние телеком-протоколы**
   - WiFi 6 (802.11ax) — AP management, client tracking
   - VoIP — Asterisk AMI, RTP monitoring, ALSA audio quality
   - IPTV — IGMP multicast, ip mroute, channel management
   - GPON — ONU management, rtl_gpon CLI
   - TR-069 — ACS monitoring, cwmpd daemon

4. **gRPC API design**
   - Protobuf specification
   - Streaming telemetry
   - Error handling
   - Service interface design

5. **Build system**
   - CMake для cross-compilation
   - protobuf/gRPC code generation
   - Install targets

### Почему это показывает уровень Middle+/Senior:

- **Не просто "код работает"**, а продуманная архитектура с HAL
- **Понимание Linux internals** — sysfs, procfs, hostapd, Asterisk, ALSA
- **Thread safety** — mutex, atomic operations
- **Testability** — mock mode для unit-тестов
- **Production-ready** — graceful shutdown, error handling
- **Residential telecom domain** — WiFi 6, VoIP, IPTV, GPON, TR-069

---

## Что можно добавить для портфолио

1. **Unit-тесты** через Google Test
2. **CI/CD** через GitHub Actions
3. **Docker** для containerization
4. **Prometheus metrics** для мониторинга
5. **TLS/mTLS** для security
6. **Configuration file** для настройки
7. **Logging** через spdlog или glog

---

## Архитектурные диаграммы (C1-C4)

### C1: Общая архитектура системы (Overall System Architecture)

Показывает внешние системы управления, gRPC сервер, HAL слой, Linux kernel subsystems и hardware платформу MT7981 + RTL960x.

![](diagrams/C1_Overall_System_Architecture.png)

**Ссылка на PlantUML источник:** [C1_overall_system_architecture.puml](diagrams/C1_overall_system_architecture.puml)

**Описание:**
- **Внешние системы**: NMS/OSS, мониторинг — подключаются к gRPC серверу через protobuf
- **gRPC Server**: основной интерфейс API на порту 50054, реализует MtsResidentialService
- **HAL Layer**: пять независимых модуля — WifiHal, VoipHal, IptvHal, GponHal, Tr069Hal
- **Linux Kernel**: 802.11 Driver, Asterisk, ALSA, IGMP Proxy, GPON Driver, cwmpd daemon
- **Hardware**: MT7981 SoC (WiFi 6 integrated), RTL960x GPON PHY, 4x GE Ports, VoIP PHY
- **Connected Devices**: 20 WiFi clients, VoIP phones, IPTV set-top boxes
- **Протоколы**: SNMP, TR-069, IPTV, Telemetry — экспортируют данные из gRPC

### C2: gRPC Service RPC Methods

Описывает все 15 RPC методов (14 unary + 1 streaming), их request/response сообщения и взаимодействие с HAL.

![](diagrams/C2_gRPC_Service_RPC_Methods.png)

**Ссылка на PlantUML источник:** [C2_grpc_service_rpc_methods.puml](diagrams/C2_grpc_service_rpc_methods.puml)

**Описание:**
- **WiFi RPC (4)**: GetWifiStatus, SetWifiConfig, GetWifiClients, SetWifiChannel
- **VoIP RPC (4)**: GetVoipStatus, GetVoipCalls, CreateVoipCall, DeleteVoipCall
- **IPTV RPC (4)**: GetIptvStatus, GetIptvChannels, AddIptvChannel, RemoveIptvChannel
- **GPON RPC (2)**: GetGponStatus, GetGponOnu
- **TR-069 RPC (2)**: GetTr069Status, GetACSInfo
- **Streaming RPC (1)**: SubscribeTelemetry (all HAL)

### C3: HAL Layer Architecture

Детальная архитектура пяти HAL модулей с их интерфейсами, структурами данных и интеграцией с Linux.

![](diagrams/C3_HAL_Layer_Architecture.png)

**Ссылка на PlantUML источник:** [C3_hal_layer_architecture.puml](diagrams/C3_hal_layer_architecture.puml)

**Описание:**
- **WifiHal (WiFi 6 Management)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `getClientList()`, `setChannel()`, `isAvailable()`
  - Структура: `WifiStatus` — interface_name, ssid, channel, frequency, mode, tx_power, signal_avg, client_count
  - Интеграция: sysfs `/sys/class/ieee80211/`, hostapd logs, iw CLI
- **VoipHal (VoIP/Telephony)**:
  - Интерфейсы: `getStatus()`, `getCallList()`, `createCall()`, `deleteCall()`, `isAvailable()`
  - Структура: `VoipCallInfo` — call_id, caller, callee, duration, codec, rtp_stream, mos_score, jitter
  - Интеграция: Asterisk AMI, ALSA, `/proc/net/udp`
- **IptvHal (IPTV Management)**:
  - Интерфейсы: `getStatus()`, `getChannelList()`, `addChannel()`, `removeChannel()`, `isAvailable()`
  - Структура: `IptvChannelInfo` — channel_id, name, multicast_address/port, video/audio_codec, bitrate
  - Интеграция: `/proc/net/igmp`, ip mroute, igmpproxy
- **GponHal (GPON ONU)**:
  - Интерфейсы: `getStatus()`, `getOnuList()`, `isAvailable()`
  - Структура: `GponStatus` — onu_count, max_onu, uplink/downlink_rate, rx_power
  - Интеграция: sysfs `/sys/class/gpon/`, rtl_gpon CLI
- **Tr069Hal (TR-069/ACS)**:
  - Интерфейсы: `getStatus()`, `getACSInfo()`, `isAvailable()`
  - Структура: `Tr069Status` — acs_count, acs_active, acs_address/port, connection_requests
  - Интеграция: `/proc/net/tcp/`, cwmpd daemon

### C4: Linux Integration & Data Flow

Показывает полный путь данных от hardware MT7981 + RTL960x через Linux kernel к внешним клиентам.

![](diagrams/C4_Linux_Integration_Data_Flow.png)

**Ссылка на PlantUML источник:** [C4_linux_integration_data_flow.puml](diagrams/C4_linux_integration_data_flow.puml)

**Описание:**
- **Путь данных (Hardware → Client)**:
  1. MT7981 + RTL960x hardware (WiFi 6, GPON PHY) → Linux kernel drivers
  2. Kernel (802.11, Asterisk, ALSA, IGMP, GPON, cwmpd) → sysfs/procfs
  3. sysfs/procfs → HAL (WifiHal, VoipHal, IptvHal, GponHal, Tr069Hal)
  4. HAL → gRPC service (MtsResidentialServiceImpl)
  5. gRPC → External clients (NMS/OSS)
- **Путь управления (Client → Hardware)**:
  1. Client → gRPC commands (SetWifiConfig, CreateVoipCall, AddIptvChannel)
  2. gRPC → HAL configuration methods
  3. HAL → kernel hostapd/iw/AMI/iproute2
  4. Kernel → Hardware configuration
- **Ключевые subsystems**:
  - 802.11 Driver — WiFi 6 frame processing
  - Asterisk — VoIP call management
  - ALSA — audio quality monitoring
  - IGMP Proxy — multicast group management
  - GPON Driver — GPON frame processing
  - cwmpd — TR-069 ACS daemon

---

## Контакты

Для вопросов по проекту:
- Email: [arifulin@gmail.com]
- GitHub: [https://github/artyomich/mts-router]
