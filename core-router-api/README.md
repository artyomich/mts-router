# MTS-CR-9000 Core Router — gRPC API Backend

## Описание проекта

Реализация **gRPC backend на C++** для маршрутизатора **MTS-CR-9000** (Core Router).
Показывает уровень владения **Linux subsystems**, **C++ API design** и **магистральными телеком-протоколами**.

---

## Что реализовано

### 1. Hardware Abstraction Layer (HAL)

Четыре независимых HAL модуля, демонстрирующих работу с Linux subsystem:

#### TofinoHal (Intel Tofino 2 ASIC)
- **bfrt_cli**: чтение состояния P4 pipeline, таблиц, counters через `bfrt_cli`
- **P4 Runtime**: управление P4 pipeline программно через P4 Runtime API
- **Чтение из sysfs**: `/sys/class/thermal/` — температура Tofino 2 ASIC
- **Чтение из /proc/net/dev**: статистика 64x 400G и 128x 100G портов
- **Мониторинг**: pipeline utilization, table utilization, packet buffer
- **Thread-safe**: mutex для защиты pipeline state и table state
- **Мок-режим**: для тестирования без hardware

#### FabricHal (Fabric Interconnect)
- **Чтение из sysfs**: `/sys/class/thermal/`, `/sys/class/net/` — fabric topology
- **bfrt_cli**: управление fabric configuration
- **Thread-safe**: mutex для защиты fabric state
- **Мок-режим**: для тестирования

#### LineCardHal (Line Card Management)
- **Чтение из sysfs**: `/sys/class/thermal/` — card temperature
- **Чтение из /proc/net**: статистика card ports
- **bfrt_cli**: управление line card configuration
- **Thread-safe**: mutex для защиты card state
- **Мок-режим**: для тестирования

#### PortHal (Port Management)
- **Чтение из sysfs**: `/sys/class/net/ethX/` — port state, speed, duplex
- **Чтение из /proc/net/dev**: per-port counters (rx/tx bytes/packets/errors)
- **iproute2**: управление port configuration
- **Thread-safe**: mutex для защиты port table
- **Мок-режим**: для тестирования

### 2. Service Layer

#### BgpMonitor (BGP Monitoring)
- Мониторинг BGP peers state, uptime, prefixes
- Управление BGP configuration через FRRouting
- Thread-safe: mutex для защиты peer table

#### Srv6Manager (SRv6 Management)
- Управление SRv6 segments (SID, actions, next-hop)
- Синхронизация с Tofino 2 pipeline
- Thread-safe: mutex для защиты segment table

#### MplsLspManager (MPLS LSP Management)
- Управление MPLS LSP state, label mapping
- Мониторинг LSP counters (bytes, packets)
- Thread-safe: mutex для защиты LSP table

#### P4RuntimeManager (P4 Runtime Management)
- Управление P4 pipeline configuration
- Чтение/запись P4 tables через P4 Runtime API
- Thread-safe: mutex для защиты pipeline state

### 3. gRPC Service

Реализация **MtsCoreRouterService** с 19 RPC методами:

| RPC | Назначение | HAL |
|-----|-----------|-----|
| `GetBgpStatus` | Получить статус BGP | BgpMonitor |
| `GetBgpPeers` | Получить список BGP peers | BgpMonitor |
| `SetBgpConfig` | Установить конфигурацию BGP | BgpMonitor |
| `AddBgpPeer` | Добавить BGP peer | BgpMonitor |
| `DeleteBgpPeer` | Удалить BGP peer | BgpMonitor |
| `GetSrv6Status` | Получить статус SRv6 | Srv6Manager |
| `GetSrv6Segments` | Получить список SRv6 segments | Srv6Manager |
| `AddSrv6Segment` | Добавить SRv6 segment | Srv6Manager |
| `DeleteSrv6Segment` | Удалить SRv6 segment | Srv6Manager |
| `GetMplsStatus` | Получить статус MPLS | MplsLspManager |
| `GetMplsLsp` | Получить список MPLS LSP | MplsLspManager |
| `CreateMplsLsp` | Создать MPLS LSP | MplsLspManager |
| `DeleteMplsLsp` | Удалить MPLS LSP | MplsLspManager |
| `GetP4Status` | Получить статус P4 Runtime | P4RuntimeManager |
| `GetP4Pipeline` | Получить статус P4 pipeline | P4RuntimeManager |
| `SetP4Pipeline` | Установить P4 pipeline | P4RuntimeManager |
| `GetP4Tables` | Получить список P4 tables | P4RuntimeManager |
| `SetP4Table` | Установить P4 table entry | P4RuntimeManager |
| `SubscribeTelemetry` | Streaming telemetry | Все HAL |

### 4. Linux Integration

- **bfrt_cli**: управление P4 pipeline, таблицами, counters через Bit Force Runtime CLI
- **P4 Runtime API**: программное управление P4 pipeline configuration
- **sysfs**: прямое чтение из `/sys/class/thermal/`, `/sys/class/net/`
- **procfs**: парсинг `/proc/net/dev`, `/proc/net/tcp`, `/proc/net/mpls`
- **FRRouting**: BGP/OSPF/MPLS daemon monitoring и configuration
- **iproute2**: port management
- **signal handling**: graceful shutdown при SIGINT/SIGTERM

### 5. C++ Design Patterns

- **RAII**: управление ресурсами через destructors
- **Thread safety**: mutex для защиты общих данных
- **Interface segregation**: HAL как interface
- **Mock mode**: для тестирования без hardware
- **Smart pointers**: unique_ptr, shared_ptr

---

## Структура проекта

```
core-router-api/
├── proto/
│   └── mts_core_router.proto          — gRPC protobuf specification
├── include/
│   ├── hal/
│   │   ├── tofino_hal.h               — Tofino 2 HAL interface
│   │   ├── fabric_hal.h               — Fabric HAL interface
│   │   ├── line_card_hal.h            — Line Card HAL interface
│   │   └── port_hal.h                 — Port HAL interface
│   ├── service/
│   │   └── core_router_service.h      — gRPC service interface
│   ├── bgp/
│   │   └── bgp_monitor.h              — BGP monitoring
│   ├── srv6/
│   │   └── srv6_manager.h             — SRv6 management
│   ├── mpls/
│   │   └── lsp_manager.h              — MPLS LSP management
│   └── p4runtime/
│       └── p4_manager.h               — P4 Runtime management
├── src/
│   ├── hal/
│   │   ├── tofino_hal.cpp             — Tofino 2 HAL implementation
│   │   ├── fabric_hal.cpp             — Fabric HAL implementation
│   │   ├── line_card_hal.cpp          — Line Card HAL implementation
│   │   └── port_hal.cpp               — Port HAL implementation
│   ├── service/
│   │   └── core_router_service.cpp    — gRPC service implementation
│   ├── bgp/
│   │   └── bgp_monitor.cpp            — BGP monitoring
│   ├── srv6/
│   │   └── srv6_manager.cpp           — SRv6 management
│   ├── mpls/
│   │   └── lsp_manager.cpp            — MPLS LSP management
│   ├── p4runtime/
│   │   └── p4_manager.cpp             — P4 Runtime management
│   └── main.cpp                       — Server entry point
├── diagrams/
│   ├── C1_overall_system_architecture.puml
│   ├── C2_grpc_service_rpc_methods.puml
│   ├── C3_hal_layer_architecture.puml
│   └── C4_linux_integration_data_flow.puml
├── tests/
│   ├── test_tofino_hal.cpp            — Tofino HAL tests
│   ├── test_fabric_hal.cpp            — Fabric HAL tests
│   ├── test_line_card_hal.cpp         — Line Card HAL tests
│   ├── test_port_hal.cpp              — Port HAL tests
│   ├── test_bgp_monitor.cpp           — BGP monitor tests
│   ├── test_srv6_manager.cpp          — SRv6 manager tests
│   ├── test_mpls_lsp_manager.cpp      — MPLS LSP manager tests
│   ├── test_p4_manager.cpp            — P4 Runtime manager tests
│   └── CMakeLists.txt
├── config/
│   └── mts-cr9000.conf                — Server configuration
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
cd core-router-api
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Запуск
```bash
./mts-core-router-server 0.0.0.0:50056
```

### Тестирование через gRPC CLI
```bash
# Get BGP status
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetBgpStatus

# Get BGP peers
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetBgpPeers

# Get SRv6 status
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetSrv6Status

# Get SRv6 segments
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetSrv6Segments

# Get MPLS status
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetMplsStatus

# Get MPLS LSP
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetMplsLsp

# Get P4 Runtime status
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetP4Status

# Get P4 pipeline
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetP4Pipeline

# Get P4 tables
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetP4Tables

# Get port status
grpcurl -plaintext localhost:50056 mts.corerouter.v1.MtsCoreRouterService/GetPortStatus

# Subscribe to telemetry
grpcurl -plaintext -d '{"paths":["cpu","memory","temperature"]}' \
    localhost:50056 mts.corerouter.v1.MtsCoreRouterService/SubscribeTelemetry
```

---

## Примеры использования

### Python клиент
```python
import grpc
import mts_core_router_pb2
import mts_core_router_pb2_grpc

channel = grpc.insecure_channel('localhost:50056')
stub = mts_core_router_pb2_grpc.MtsCoreRouterServiceStub(channel)

# Get BGP status
response = stub.GetBgpStatus(mts_core_router_pb2.Empty())
print(f"BGP Peers: {response.bgp_status.peer_count}")
print(f"Active sessions: {response.bgp_status.active_sessions}")
print(f"Prefixes received: {response.bgp_status.prefixes_received}")
print(f"Prefixes sent: {response.bgp_status.prefixes_sent}")

# Get BGP peers
response = stub.GetBgpPeers(mts_core_router_pb2.Empty())
for peer in response.peers:
    print(f"Peer: {peer.peer_address} ASN:{peer.peer_asn} State:{peer.state}")
    print(f"  Uptime: {peer.uptime} s, Prefixes: {peer.prefixes_received}/{peer.prefixes_sent}")

# Get SRv6 segments
response = stub.GetSrv6Segments(mts_core_router_pb2.Empty())
for seg in response.segments:
    print(f"SID {seg.sid_index}: {seg.sid_value} -> {seg.next_hop} ({seg.action})")

# Add SRv6 segment
response = stub.AddSrv6Segment(
    mts_core_router_pb2.AddSrv6SegmentRequest(
        sid_index=100,
        sid_value="2001:db8::1",
        action="ENCAPSULATE",
        next_hop="192.168.1.1"
    )
)
print(f"Segment added: {response.success}")

# Get P4 pipeline
response = stub.GetP4Pipeline(mts_core_router_pb2.Empty())
print(f"Pipeline: {response.pipeline_info.program_name}")
print(f"Tables: {response.pipeline_info.table_count}")
print(f"Counters: {response.pipeline_info.counter_count}")
print(f"Registers: {response.pipeline_info.register_count}")

# Get P4 tables
response = stub.GetP4Tables(mts_core_router_pb2.Empty())
for table in response.tables:
    print(f"Table: {table.name}, Entries: {table.entry_count}")

# Get port status
response = stub.GetPortStatus(mts_core_router_pb2.Empty())
for port in response.ports:
    print(f"Port {port.port_id}: {port.port_name} {port.speed} {port.oper_state}")
    print(f"  RX: {port.rx_bytes} bytes, TX: {port.tx_bytes} bytes")

# Subscribe to telemetry
responses = stub.SubscribeTelemetry(
    mts_core_router_pb2.TelemetrySubscription(
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
    pb "path/to/mts_core_router"
)

func main() {
    conn, _ := grpc.Dial("localhost:50056", grpc.WithInsecure())
    defer conn.Close()
    
    client := pb.NewMtsCoreRouterServiceClient(conn)
    
    // Get BGP status
    resp, _ := client.GetBgpStatus(context.Background(), &pb.Empty{})
    fmt.Printf("BGP Peers: %d\n", resp.BgpStatus.PeerCount)
    fmt.Printf("Active sessions: %d\n", resp.BgpStatus.ActiveSessions)
    fmt.Printf("Prefixes received: %d\n", resp.BgpStatus.PrefixesReceived)
    fmt.Printf("Prefixes sent: %d\n", resp.BgpStatus.PrefixesSent)
    
    // Get BGP peers
    resp, _ = client.GetBgpPeers(context.Background(), &pb.Empty{})
    for _, peer := range resp.Peers {
        fmt.Printf("Peer: %s ASN:%d State:%s\n", peer.PeerAddress, peer.PeerAsn, peer.State)
        fmt.Printf("  Uptime: %d s, Prefixes: %d/%d\n", peer.Uptime, peer.PrefixesReceived, peer.PrefixesSent)
    }
    
    // Get SRv6 segments
    resp, _ = client.GetSrv6Segments(context.Background(), &pb.Empty{})
    for _, seg := range resp.Segments {
        fmt.Printf("SID %d: %s -> %s (%s)\n", seg.SidIndex, seg.SidValue, seg.NextHop, seg.Action)
    }
    
    // Get P4 pipeline
    resp, _ = client.GetP4Pipeline(context.Background(), &pb.Empty{})
    fmt.Printf("Pipeline: %s\n", resp.PipelineInfo.ProgramName)
    fmt.Printf("Tables: %d\n", resp.PipelineInfo.TableCount)
    fmt.Printf("Counters: %d\n", resp.PipelineInfo.CounterCount)
    fmt.Printf("Registers: %d\n", resp.PipelineInfo.RegisterCount)
    
    // Get P4 tables
    resp, _ = client.GetP4Tables(context.Background(), &pb.Empty{})
    for _, table := range resp.Tables {
        fmt.Printf("Table: %s, Entries: %d\n", table.Name, table.EntryCount)
    }
    
    // Get port status
    resp, _ = client.GetPortStatus(context.Background(), &pb.Empty{})
    for _, port := range resp.Ports {
        fmt.Printf("Port %d: %s %s %s\n", port.PortId, port.PortName, port.Speed, port.OperState)
        fmt.Printf("  RX: %d bytes, TX: %d bytes\n", port.RxBytes, port.TxBytes)
    }
    
    // Subscribe to telemetry
    responses, _ := client.SubscribeTelemetry(context.Background(), &pb.TelemetrySubscription{
        Paths: []string{"cpu", "memory", "temperature"},
        SampleInterval: 1000,
    })
    for resp := range responses {
        fmt.Printf("Timestamp: %d\n", resp.Timestamp)
        fmt.Printf("Metrics: %s\n", resp.Metrics)
    }
}
```

---

## Уровень реализации для портфолио

### Что показывает этот код:

1. **Работа с Linux kernel interfaces**
   - bfrt_cli — Intel Tofino 2 P4 pipeline management
   - P4 Runtime API — programmable data plane configuration
   - sysfs (`/sys/class/thermal/`, `/sys/class/net/`)
   - procfs (`/proc/net/dev`, `/proc/net/tcp`, `/proc/net/mpls`)
   - FRRouting — BGP/OSPF/MPLS daemon management
   - iproute2 — port management

2. **C++ на уровне embedded**
   - Thread-safe HAL через mutex
   - RAII для управления ресурсами
   - Smart pointers (unique_ptr, shared_ptr)
   - Mock mode для тестирования
   - Signal handling (SIGINT, SIGTERM)

3. **Магистральные телеком-протоколы**
   - BGP-4/6 — peer management, prefix tracking, session state
   - SRv6 — Segment Routing IPv6, SID management, encapsulation
   - MPLS — LSP state, label mapping, forwarding
   - P4 Runtime — programmable data plane, pipeline configuration

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
- **Понимание Linux internals** — sysfs, procfs, bfrt_cli, P4 Runtime
- **Thread safety** — mutex, atomic operations
- **Testability** — mock mode для unit-тестов
- **Production-ready** — graceful shutdown, error handling
- **Core router domain** — BGP, SRv6, MPLS, P4 Runtime

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

Показывает внешние системы управления, SDN controller, WAN networks, gRPC сервер, service layer, HAL слой, Linux kernel subsystems и hardware платформу Intel Tofino 2 + AMD EPYC.

![](diagrams/C1_Overall_System_Architecture.png)

**Ссылка на PlantUML источник:** [C1_overall_system_architecture.puml](diagrams/C1_overall_system_architecture.puml)

**Описание:**
- **Внешние системы**: NMS/OSS, мониторинг, SDN Controller — подключаются к gRPC/P4 Runtime серверу
- **WAN Networks**: BGP Peers, Transit Providers, Data Center Spine
- **gRPC Server**: основной интерфейс API на порту 50056, реализует MtsCoreRouterService
- **Service Layer**: BGP Monitor, SRv6 Manager, MPLS LSP Manager, P4 Runtime Manager
- **HAL Layer**: четыре независимых модуля — TofinoHal, FabricHal, LineCardHal, PortHal
- **Linux Kernel**: bfrt_cli, P4 Runtime, FRRouting, network stack
- **Hardware**: Intel Tofino 2 ASIC (P4 programmable), AMD EPYC 7003 CPU, 64x 400G SFP-DD, 128x 100G OSFP, NVMe SSD, DDR5 ECC
- **Протоколы**: BGP/4/6, NETCONF/YANG, P4Runtime, Telemetry — экспортируют данные из gRPC

### C2: gRPC Service RPC Methods

Описывает все 19 RPC методов (18 unary + 1 streaming), их request/response сообщения и взаимодействие с HAL.

![](diagrams/C2_gRPC_Service_RPC_Methods.png)

**Ссылка на PlantUML источник:** [C2_grpc_service_rpc_methods.puml](diagrams/C2_grpc_service_rpc_methods.puml)

**Описание:**
- **BGP RPC (5)**: GetBgpStatus, GetBgpPeers, SetBgpConfig, AddBgpPeer, DeleteBgpPeer
- **SRv6 RPC (4)**: GetSrv6Status, GetSrv6Segments, AddSrv6Segment, DeleteSrv6Segment
- **MPLS RPC (4)**: GetMplsStatus, GetMplsLsp, CreateMplsLsp, DeleteMplsLsp
- **P4 Runtime RPC (5)**: GetP4Status, GetP4Pipeline, SetP4Pipeline, GetP4Tables, SetP4Table
- **Port RPC (2)**: GetPortStatus, SetPortConfig
- **Streaming RPC (1)**: SubscribeTelemetry (all HAL)

### C3: HAL Layer Architecture

Детальная архитектура четырёх HAL модулей с их интерфейсами, структурами данных и интеграцией с Linux.

![](diagrams/C3_HAL_Layer_Architecture.png)

**Ссылка на PlantUML источник:** [C3_hal_layer_architecture.puml](diagrams/C3_hal_layer_architecture.puml)

**Описание:**
- **TofinoHal (Intel Tofino 2 ASIC)**:
  - Интерфейсы: `getStatus()`, `getPipelineStatus()`, `setPipeline()`, `getTableList()`, `setTable()`, `isAvailable()`
  - Структура: `TofinoStatus` — pipeline_utilization, table_utilization, packet_buffer_used/max, forwarding_entries, pipeline_clock, thermal_zone
  - Интеграция: bfrt_cli, P4 Runtime API, sysfs `/sys/class/thermal/`, `/proc/net/dev`
- **FabricHal (Fabric Interconnect)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `isAvailable()`
  - Структура: `FabricStatus` — fabric_id, fabric_utilization, fabric_temperature, fabric_status
  - Интеграция: sysfs, bfrt_cli
- **LineCardHal (Line Card Management)**:
  - Интерфейсы: `getStatus()`, `getPortList()`, `setConfig()`, `isAvailable()`
  - Структура: `LineCardStatus` — card_id, port_count, active_ports, temperature, power_usage
  - Интеграция: sysfs, bfrt_cli
- **PortHal (Port Management)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `isAvailable()`
  - Структура: `PortStatus` — port_id, port_name, speed, duplex, admin/oper_state, rx/tx bytes/packets/errors
  - Интеграция: sysfs `/sys/class/net/`, procfs `/proc/net/dev`, iproute2

### C4: Linux Integration & Data Flow

Показывает полный путь данных от hardware Intel Tofino 2 + AMD EPYC через Linux kernel к внешним клиентам.

![](diagrams/C4_Linux_Integration_Data_Flow.png)

**Ссылка на PlantUML источник:** [C4_linux_integration_data_flow.puml](diagrams/C4_linux_integration_data_flow.puml)

**Описание:**
- **Путь данных (Hardware → Client)**:
  1. Intel Tofino 2 + AMD EPYC hardware (400G/100G MAC) → Linux kernel drivers
  2. Kernel (bfrt_cli, P4 Runtime, FRRouting) → sysfs/procfs
  3. sysfs/procfs → HAL (TofinoHal, FabricHal, LineCardHal, PortHal)
  4. HAL → Service Layer (BGP/SRv6/MPLS/P4 managers)
  5. Service → gRPC server (MtsCoreRouterServiceImpl)
  6. gRPC → External clients (NMS/OSS/SDN)
- **Путь управления (Client → Hardware)**:
  1. Client/SDN → gRPC/P4 Runtime commands (SetBgpConfig, AddSrv6Segment, SetP4Pipeline)
  2. gRPC/P4 Runtime → Service Layer
  3. Service → HAL configuration methods
  4. HAL → kernel bfrt_cli/P4 Runtime
  5. Kernel → Hardware configuration
- **Ключевые subsystems**:
  - bfrt_cli — Intel Tofino 2 P4 pipeline management
  - P4 Runtime — programmable data plane API
  - FRRouting — BGP/OSPF/MPLS daemon
  - Network Subsystem — packet processing

---

## Контакты

Для вопросов по проекту:
- Email: [arifulin@gmail.com]
- GitHub: [https://github/artyomich/mts-router]
