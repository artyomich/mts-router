# MTS-MC-5000 Mobile Core — gRPC API Backend

## Описание проекта

Реализация **gRPC backend на C++** для маршрутизатора **MTS-MC-5000** (Mobile Core 5G).
Показывает уровень владения **Linux subsystems**, **C++ API design** и **5G core протоколами**.

---

## Что реализовано

### 1. Hardware Abstraction Layer (HAL)

Четыре независимых HAL модуля, демонстрирующих работу с Linux subsystem:

#### UpfHal (UPF — User Plane Function)
- **Чтение из procfs**: `/proc/net/nf_conntrack` — conntrack entries, session counters
- **Чтение из sysfs**: `/sys/class/net/ethX/` — port statistics, interface state
- **nftables**: управление правилами NAT и steering через `nft` CLI
- **Thread-safe**: mutex для защиты conntrack table и nft rules
- **Мок-режим**: для тестирования без hardware

#### SmfHal (SMF — Session Management Function)
- **Чтение из sysfs**: `/sys/class/net/ethX/` — DNS config, PGW address
- **iptables**: управление правилами маршрутизации через `iptables` CLI
- **Парсинг /proc/net**: статистика network интерфейсов
- **Thread-safe**: mutex для защиты iptables rules
- **Мок-режим**: для тестирования

#### GtpHal (GTP-U — GPRS Tunnelling Protocol)
- **Чтение из sysfs**: `/sys/class/net/ethX/` — GTP-U tunnel stats
- **Чтение из /proc/net**: статистика GTP-U tunnels
- **ioctl**: настройка GTP-U tunnel parameters
- **Thread-safe**: mutex для защиты tunnel table
- **Мок-режим**: для тестирования

#### PfcpHal (PFCP — PFCP Session Management)
- **Чтение из /proc/net**: PFCP session stats, FAR/QER counters
- **nftables**: управление steering rules через `nft` CLI
- **iproute2**: управление MPLS pseudowires
- **Thread-safe**: mutex для защиты rule table
- **Мок-режим**: для тестирования

### 2. gRPC Service

Реализация **MtsMobileCoreService** с 16 RPC методами:

| RPC | Назначение | HAL |
|-----|-----------|-----|
| `GetUpfStatus` | Получить статус UPF | UpfHal |
| `SetUpfConfig` | Установить конфигурацию UPF | UpfHal |
| `GetUpfSessions` | Получить список UPF сессий | UpfHal |
| `CreateUpfSession` | Создать UPF сессию | UpfHal |
| `DeleteUpfSession` | Удалить UPF сессию | UpfHal |
| `GetSmfStatus` | Получить статус SMF | SmfHal |
| `SetSmfConfig` | Установить конфигурацию SMF | SmfHal |
| `GetSmfSessions` | Получить список SMF сессий | SmfHal |
| `CreateSmfSession` | Создать SMF сессию | SmfHal |
| `GetGtpStatus` | Получить статус GTP-U | GtpHal |
| `GetGtpTunnels` | Получить список GTP-U туннелей | GtpHal |
| `CreateGtpTunnel` | Создать GTP-U туннель | GtpHal |
| `DeleteGtpTunnel` | Удалить GTP-U туннель | GtpHal |
| `GetPfcpStatus` | Получить статус PFCP | PfcpHal |
| `SetPfcpRules` | Установить PFCP правила | PfcpHal |
| `SubscribeTelemetry` | Streaming telemetry | Все HAL |

### 3. Linux Integration

- **sysfs**: прямое чтение из `/sys/class/net/`, `/sys/class/thermal/`
- **procfs**: парсинг `/proc/net/nf_conntrack`, `/proc/net/dev`, `/proc/net/tcp`
- **nftables**: управление правилами NAT и steering
- **iptables**: управление правилами маршрутизации
- **ioctl**: настройка GTP-U tunnel parameters
- **iproute2**: управление MPLS pseudowires
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
mobile-core-api/
├── proto/
│   └── mts_mobile_core.proto          — gRPC protobuf specification
├── include/
│   ├── hal/
│   │   ├── upf_hal.h                  — UPF HAL interface
│   │   ├── sm_hal.h                   — SMF HAL interface
│   │   ├── gtp_hal.h                  — GTP-U HAL interface
│   │   └── pfcp_hal.h                 — PFCP HAL interface
│   ├── service/
│   │   └── mobile_core_service.h      — gRPC service interface
│   ├── upf/
│   │   └── session_manager.h          — UPF session management
│   ├── smf/
│   │   └── session_controller.h       — SMF session controller
│   ├── gtp/
│   │   └── gtp_tunnel.h               — GTP-U tunnel management
│   └── pfcp/
│       └── pfcp_handler.h             — PFCP session handler
├── src/
│   ├── hal/
│   │   ├── upf_hal.cpp                — UPF HAL implementation
│   │   ├── sm_hal.cpp                 — SMF HAL implementation
│   │   ├── gtp_hal.cpp                — GTP-U HAL implementation
│   │   └── pfcp_hal.cpp               — PFCP HAL implementation
│   ├── service/
│   │   └── mobile_core_service.cpp    — gRPC service implementation
│   ├── upf/
│   │   └── session_manager.cpp        — UPF session management
│   ├── smf/
│   │   └── session_controller.cpp     — SMF session controller
│   ├── gtp/
│   │   └── gtp_tunnel.cpp             — GTP-U tunnel management
│   ├── pfcp/
│   │   └── pfcp_handler.cpp           — PFCP session handler
│   └── main.cpp                       — Server entry point
├── diagrams/
│   ├── C1_overall_system_architecture.puml
│   ├── C2_grpc_service_rpc_methods.puml
│   ├── C3_hal_layer_architecture.puml
│   └── C4_linux_integration_data_flow.puml
├── tests/
│   ├── test_upf_hal.cpp               — UPF HAL tests
│   ├── test_sm_hal.cpp                — SMF HAL tests
│   ├── test_gtp_hal.cpp               — GTP-U HAL tests
│   ├── test_pfcp_hal.cpp              — PFCP HAL tests
│   └── CMakeLists.txt
├── config/
│   └── mts-mc5000.conf                — Server configuration
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
cd mobile-core-api
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Запуск
```bash
./mts-mobile-core-server 0.0.0.0:50052
```

### Тестирование через gRPC CLI
```bash
# Get UPF status
grpcurl -plaintext localhost:50052 mts.mobilecore.v1.MtsMobileCoreService/GetUpfStatus

# Get device health
grpcurl -plaintext localhost:50052 mts.mobilecore.v1.MtsMobileCoreService/GetUpfSessions

# Get GTP tunnels
grpcurl -plaintext localhost:50052 mts.mobilecore.v1.MtsMobileCoreService/GetGtpTunnels

# Get PFCP rules
grpcurl -plaintext localhost:50052 mts.mobilecore.v1.MtsMobileCoreService/GetPfcpStatus

# Subscribe to telemetry
grpcurl -plaintext -d '{"paths":["cpu","memory","temperature"]}' \
    localhost:50052 mts.mobilecore.v1.MtsMobileCoreService/SubscribeTelemetry
```

---

## Примеры использования

### Python клиент
```python
import grpc
import mts_mobile_core_pb2
import mts_mobile_core_pb2_grpc

channel = grpc.insecure_channel('localhost:50052')
stub = mts_mobile_core_pb2_grpc.MtsMobileCoreServiceStub(channel)

# Get UPF status
response = stub.GetUpfStatus(mts_mobile_core_pb2.Empty())
print(f"UPF Sessions: {response.upf_status.session_count}")
print(f"Uplink bytes: {response.upf_status.uplink_bytes}")
print(f"Conntrack entries: {response.upf_status.conntrack_entries}")

# Create GTP-U tunnel
response = stub.CreateGtpTunnel(
    mts_mobile_core_pb2.CreateGtpTunnelRequest(
        session_id="12345",
        teid_local=0x1000,
        teid_remote=0x2000,
        peer_address="192.168.1.100"
    )
)
print(f"Tunnel created: {response.success}")

# Subscribe to telemetry
responses = stub.SubscribeTelemetry(
    mts_mobile_core_pb2.TelemetrySubscription(
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
    pb "path/to/mts_mobile_core"
)

func main() {
    conn, _ := grpc.Dial("localhost:50052", grpc.WithInsecure())
    defer conn.Close()
    
    client := pb.NewMtsMobileCoreServiceClient(conn)
    
    // Get UPF status
    resp, _ := client.GetUpfStatus(context.Background(), &pb.Empty{})
    fmt.Printf("UPF Sessions: %d\n", resp.UpfStatus.SessionCount)
    fmt.Printf("Uplink bytes: %d\n", resp.UpfStatus.UplinkBytes)
    fmt.Printf("Conntrack entries: %d\n", resp.UpfStatus.ConntrackEntries)
    
    // Create GTP-U tunnel
    resp, _ = client.CreateGtpTunnel(context.Background(), &pb.CreateGtpTunnelRequest{
        SessionId: "12345",
        TeidLocal: 0x1000,
        TeidRemote: 0x2000,
        PeerAddress: "192.168.1.100",
    })
    fmt.Printf("Tunnel created: %t\n", resp.Success)
}
```

---

## Уровень реализации для портфолио

### Что показывает этот код:

1. **Работа с Linux kernel interfaces**
   - sysfs (`/sys/class/net/`, `/sys/class/thermal/`)
   - procfs (`/proc/net/nf_conntrack`, `/proc/net/dev`, `/proc/net/tcp`)
   - nftables — управление правилами NAT и steering
   - iptables — управление правилами маршрутизации
   - ioctl для GTP-U tunnel config
   - iproute2 для MPLS pseudowires

2. **C++ на уровне embedded**
   - Thread-safe HAL через mutex
   - RAII для управления ресурсами
   - Smart pointers (unique_ptr, shared_ptr)
   - Mock mode для тестирования
   - Signal handling (SIGINT, SIGTERM)

3. **5G core протоколы**
   - UPF — User Plane Function (conntrack, NAT, forwarding)
   - SMF — Session Management Function (DNS, PGW, session lifecycle)
   - GTP-U — GPRS Tunnelling Protocol (TEID mapping, tunnel management)
   - PFCP — PFCP Session Management (FAR/QER rules, steering)

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
- **Понимание Linux internals** — sysfs, procfs, nftables, iptables
- **Thread safety** — mutex, atomic operations
- **Testability** — mock mode для unit-тестов
- **Production-ready** — graceful shutdown, error handling
- **5G core domain** — UPF, SMF, GTP-U, PFCP

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

Показывает внешние системы управления, 5G core peers, gRPC сервер, service layer, HAL слой, Linux kernel subsystems и hardware платформу ThunderX3.

![](diagrams/C1_Overall_System_Architecture.png)

**Ссылка на PlantUML источник:** [C1_overall_system_architecture.puml](diagrams/C1_overall_system_architecture.puml)

**Описание:**
- **Внешние системы**: NMS/OSS, мониторинг — подключаются к gRPC серверу через protobuf
- **5G Core Network**: AMF, UDM, UPF peers — взаимодействуют через GTP-U и SM signaling
- **gRPC Server**: основной интерфейс API на порту 50052, реализует MtsMobileCoreService
- **Service Layer**: UPF Session Manager, SMF Session Controller, GTP Tunnel Manager, PFCP Handler
- **HAL Layer**: четыре независимых модуля — UpfHal, SmfHal, GtpHal, PfcpHal
- **Linux Kernel**: conntrack, nftables, IPTables, GTP-U subsystem, network stack
- **Hardware**: ThunderX3 SoC с 100G SFP28 и 25G SFP28 портами
- **Протоколы**: SNMP, NETCONF/YANG, Telemetry — экспортируют данные из gRPC

### C2: gRPC Service RPC Methods

Описывает все 16 RPC методов (15 unary + 1 streaming), их request/response сообщения и взаимодействие с HAL.

![](diagrams/C2_gRPC_Service_RPC_Methods.png)

**Ссылка на PlantUML источник:** [C2_grpc_service_rpc_methods.puml](diagrams/C2_grpc_service_rpc_methods.puml)

**Описание:**
- **Unary RPC (15 методов)**:
  - UPF RPC (5): GetUpfStatus, SetUpfConfig, GetUpfSessions, CreateUpfSession, DeleteUpfSession
  - SMF RPC (4): GetSmfStatus, SetSmfConfig, GetSmfSessions, CreateSmfSession
  - GTP RPC (4): GetGtpStatus, GetGtpTunnels, CreateGtpTunnel, DeleteGtpTunnel
  - PFCP RPC (3): GetPfcpStatus, SetPfcpRules, GetPfcpRules
- **Streaming RPC (1 метод)**:
  - SubscribeTelemetry — streaming telemetry от всех HAL

### C3: HAL Layer Architecture

Детальная архитектура четырёх HAL модулей с их интерфейсами, структурами данных и интеграцией с Linux.

![](diagrams/C3_HAL_Layer_Architecture.png)

**Ссылка на PlantUML источник:** [C3_hal_layer_architecture.puml](diagrams/C3_hal_layer_architecture.puml)

**Описание:**
- **UpfHal (User Plane Function)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `getSessionList()`, `isAvailable()`
  - Структура: `UpfStatus` — session_count, uplink/downlink bytes/packets, conntrack_entries, nft_rules_count
  - Интеграция: sysfs `/sys/class/net/`, procfs `/proc/net/nf_conntrack`, nftables
- **SmfHal (Session Management Function)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `getSessionList()`, `isAvailable()`
  - Структура: `SmfStatus` — session_count, dns_entries, pgw_address, amf_address
  - Интеграция: sysfs `/sys/class/net/`, procfs `/proc/net/`, iptables
- **GtpHal (GTP-U Tunnel Manager)**:
  - Интерфейсы: `getStatus()`, `getTunnelList()`, `createTunnel()`, `deleteTunnel()`, `isAvailable()`
  - Структура: `GtpTunnelStatus` — teid_local/remote, peer_address, session_id, qos_class, counters
  - Интеграция: sysfs `/sys/class/net/`, procfs `/proc/net/`, ioctl
- **PfcpHal (PFCP Session Handler)**:
  - Интерфейсы: `getStatus()`, `setRules()`, `getRules()`, `isAvailable()`
  - Структура: `PfcpRuleStatus` — rule_id, far_id, qer_id, counters, steering_action
  - Интеграция: procfs `/proc/net/`, nftables, iproute2

### C4: Linux Integration & Data Flow

Показывает полный путь данных от hardware ThunderX3 через Linux kernel к внешним клиентам.

![](diagrams/C4_Linux_Integration_Data_Flow.png)

**Ссылка на PlantUML источник:** [C4_linux_integration_data_flow.puml](diagrams/C4_linux_integration_data_flow.puml)

**Описание:**
- **Путь данных (Hardware → Client)**:
  1. ThunderX3 hardware (100G/25G MAC) → PCIe → Linux kernel drivers
  2. Kernel (conntrack, nftables, GTP-U, IPTables) → sysfs/procfs
  3. sysfs/procfs → HAL (UpfHal, SmfHal, GtpHal, PfcpHal)
  4. HAL → Service Layer (UPF/SMF/GTP/PFCP managers)
  5. Service → gRPC server (MtsMobileCoreServiceImpl)
  6. gRPC → External clients (NMS/OSS)
- **Путь управления (Client → Hardware)**:
  1. Client → gRPC commands (SetUpfConfig, CreateGtpTunnel, SetPfcpRules)
  2. gRPC → Service Layer
  3. Service → HAL configuration methods
  4. HAL → kernel ioctl/nft/iptables
  5. Kernel → Hardware configuration
- **Ключевые subsystems**:
  - conntrack — connection tracking for UPF sessions
  - nftables — packet steering and NAT rules
  - IPTables — routing rules for SMF
  - GTP-U — tunnel management
  - Network Subsystem — packet processing

---

## Контакты

Для вопросов по проекту:
- Email: [arifulin@gmail.com]
- GitHub: [https://github/artyomich/mts-router]
