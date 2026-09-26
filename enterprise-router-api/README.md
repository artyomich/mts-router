# MTS-ER-1000 Enterprise Router — gRPC API Backend

## Описание проекта

Реализация **gRPC backend на C++** для маршрутизатора **MTS-ER-1000** (Enterprise Router).
Показывает уровень владения **Linux subsystems**, **C++ API design** и **enterprise телеком-протоколами**.

---

## Что реализовано

### 1. Hardware Abstraction Layer (HAL)

Четыре независимых HAL модуля, демонстрирующих работу с Linux subsystem:

#### SdwanHal (SD-WAN Path Management)
- **Чтение из sysfs**: `/sys/class/thermal/`, `/sys/class/net/ethX/` — температура, port stats
- **Чтение из /proc/net**: мониторинг BFD sessions, path health metrics
- **BFD monitoring**: отслеживание latency, packet loss per WAN path
- **keepalived**: мониторинг VRRP state для path failover
- **Thread-safe**: mutex для защиты WAN path table
- **Мок-режим**: для тестирования (4 WAN paths mock)

#### IpsecHal (IPsec Tunnel Management)
- **Чтение из /proc/net/xfrm_state**: IPsec SA state, byte/packet counters
- **Чтение из /proc/net/xfrm_policy**: IPsec policy state
- **strongSwan monitoring**: мониторинг IPsec daemon state
- **Thread-safe**: mutex для защиты tunnel table
- **Мок-режим**: для тестирования

#### VrrpHal (VRRP Monitoring)
- **Чтение из sysfs**: `/sys/class/thermal/`, `/sys/class/net/` — temperature, port state
- **Парсинг keepalived state**: мониторинг VRRP instances (master/backup)
- **Thread-safe**: mutex для защиты VRRP instance table
- **Мок-режим**: для тестирования (3 VRRP instances mock)

#### MplsHal (MPLS LSP Management)
- **Чтение из /proc/net/mpls**: MPLS LSP state, label mapping
- **FRRouting monitoring**: мониторинг MPLS daemon state
- **Thread-safe**: mutex для защиты LSP table
- **Мок-режим**: для тестирования (4 LSP mock)

### 2. gRPC Service

Реализация **MtsEnterpriseService** с 15 RPC методами:

| RPC | Назначение | HAL |
|-----|-----------|-----|
| `GetSdwanStatus` | Получить статус SD-WAN | SdwanHal |
| `SetSdwanConfig` | Установить конфигурацию SD-WAN | SdwanHal |
| `GetWanPaths` | Получить список WAN paths | SdwanHal |
| `SetPathPriority` | Установить приоритет path | SdwanHal |
| `GetIpsecStatus` | Получить статус IPsec | IpsecHal |
| `GetIpsecTunnels` | Получить список IPsec tunnels | IpsecHal |
| `CreateIpsecTunnel` | Создать IPsec tunnel | IpsecHal |
| `DeleteIpsecTunnel` | Удалить IPsec tunnel | IpsecHal |
| `GetVrrpStatus` | Получить статус VRRP | VrrpHal |
| `GetVrrpInstances` | Получить список VRRP instances | VrrpHal |
| `SetVrrpPriority` | Установить VRRP priority | VrrpHal |
| `GetMplsStatus` | Получить статус MPLS | MplsHal |
| `GetMplsLsp` | Получить список MPLS LSP | MplsHal |
| `CreateMplsLsp` | Создать MPLS LSP | MplsHal |
| `SubscribeTelemetry` | Streaming telemetry | Все HAL |

### 3. Linux Integration

- **sysfs**: прямое чтение из `/sys/class/thermal/`, `/sys/class/net/`
- **procfs**: парсинг `/proc/net/xfrm_state`, `/proc/net/xfrm_policy`, `/proc/net/mpls`, `/proc/net/dev`
- **BFD monitoring**: path health per WAN interface
- **keepalived**: VRRP instance state monitoring
- **strongSwan**: IPsec tunnel monitoring
- **FRRouting**: MPLS LSP monitoring
- **iproute2**: MPLS pseudowire management
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
enterprise-router-api/
├── proto/
│   └── mts_enterprise.proto           — gRPC protobuf specification
├── include/
│   ├── hal/
│   │   ├── sdwan_hal.h                — SD-WAN HAL interface
│   │   ├── ipsec_hal.h                — IPsec HAL interface
│   │   ├── vrrp_hal.h                 — VRRP HAL interface
│   │   └── mpls_hal.h                 — MPLS HAL interface
│   └── service/
│       └── enterprise_service.h       — gRPC service interface
├── src/
│   ├── hal/
│   │   ├── sdwan_hal.cpp              — SD-WAN HAL implementation
│   │   ├── ipsec_hal.cpp              — IPsec HAL implementation
│   │   ├── vrrp_hal.cpp               — VRRP HAL implementation
│   │   └── mpls_hal.cpp               — MPLS HAL implementation
│   ├── service/
│   │   └── enterprise_service.cpp     — gRPC service implementation
│   └── main.cpp                       — Server entry point
├── diagrams/
│   ├── C1_overall_system_architecture.puml
│   ├── C2_grpc_service_rpc_methods.puml
│   ├── C3_hal_layer_architecture.puml
│   └── C4_linux_integration_data_flow.puml
├── tests/
│   ├── test_sdwan_hal.cpp             — SD-WAN HAL tests
│   ├── test_ipsec_hal.cpp             — IPsec HAL tests
│   ├── test_vrrp_hal.cpp              — VRRP HAL tests
│   ├── test_mpls_hal.cpp              — MPLS HAL tests
│   └── CMakeLists.txt
├── config/
│   └── mts-er1000.conf                — Server configuration
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
cd enterprise-router-api
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Запуск
```bash
./mts-enterprise-router-server 0.0.0.0:50055
```

### Тестирование через gRPC CLI
```bash
# Get SD-WAN status
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetSdwanStatus

# Get WAN paths
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetWanPaths

# Get IPsec status
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetIpsecStatus

# Get IPsec tunnels
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetIpsecTunnels

# Get VRRP status
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetVrrpStatus

# Get VRRP instances
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetVrrpInstances

# Get MPLS status
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetMplsStatus

# Get MPLS LSP
grpcurl -plaintext localhost:50055 mts.enterprise.v1.MtsEnterpriseService/GetMplsLsp

# Subscribe to telemetry
grpcurl -plaintext -d '{"paths":["cpu","memory","temperature"]}' \
    localhost:50055 mts.enterprise.v1.MtsEnterpriseService/SubscribeTelemetry
```

---

## Примеры использования

### Python клиент
```python
import grpc
import mts_enterprise_pb2
import mts_enterprise_pb2_grpc

channel = grpc.insecure_channel('localhost:50055')
stub = mts_enterprise_pb2_grpc.MtsEnterpriseServiceStub(channel)

# Get SD-WAN status
response = stub.GetSdwanStatus(mts_enterprise_pb2.Empty())
print(f"Active WAN paths: {response.sdwan_status.active_wan_paths}")
print(f"Selected path: {response.sdwan_status.selected_path}")
print(f"BFD sessions: {response.sdwan_status.bfd_sessions}")
print(f"Path health score: {response.sdwan_status.path_health_score}")

# Get WAN paths
response = stub.GetWanPaths(mts_enterprise_pb2.Empty())
for path in response.wan_paths:
    print(f"Path {path.path_id}: {path.interface_name} -> {path.ip_address}")
    print(f"  Latency: {path.latency_ms} ms, Loss: {path.packet_loss_pct}%")
    print(f"  BW: {path.bandwidth_up}/{path.bandwidth_down} Mbps")
    print(f"  Priority: {path.priority}, Status: {path.status}")

# Get IPsec tunnels
response = stub.GetIpsecTunnels(mts_enterprise_pb2.Empty())
for tunnel in response.tunnels:
    print(f"Tunnel {tunnel.tunnel_id}: {tunnel.local_ip} -> {tunnel.remote_ip}")
    print(f"  Status: {tunnel.status}, Encryption: {tunnel.encryption}")
    print(f"  Bytes: {tunnel.bytes_in}/{tunnel.bytes_out}")

# Create IPsec tunnel
response = stub.CreateIpsecTunnel(
    mts_enterprise_pb2.CreateIpsecTunnelRequest(
        tunnel_id="tun1",
        local_ip="192.168.1.1",
        remote_ip="10.0.0.1",
        encryption="AES-256-GCM",
        auth_algo="SHA-256"
    )
)
print(f"Tunnel created: {response.success}")

# Get VRRP instances
response = stub.GetVrrpInstances(mts_enterprise_pb2.Empty())
for inst in response.instances:
    print(f"VRRP {inst.instance_id}: {inst.virtual_ip} -> {inst.state}")
    print(f"  Priority: {inst.priority}, Master: {inst.master_ip}")

# Subscribe to telemetry
responses = stub.SubscribeTelemetry(
    mts_enterprise_pb2.TelemetrySubscription(
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
    pb "path/to/mts_enterprise"
)

func main() {
    conn, _ := grpc.Dial("localhost:50055", grpc.WithInsecure())
    defer conn.Close()
    
    client := pb.NewMtsEnterpriseServiceClient(conn)
    
    // Get SD-WAN status
    resp, _ := client.GetSdwanStatus(context.Background(), &pb.Empty{})
    fmt.Printf("Active WAN paths: %d\n", resp.SdwanStatus.ActiveWanPaths)
    fmt.Printf("Selected path: %s\n", resp.SdwanStatus.SelectedPath)
    fmt.Printf("BFD sessions: %d\n", resp.SdwanStatus.BfdSessions)
    fmt.Printf("Path health score: %d\n", resp.SdwanStatus.PathHealthScore)
    
    // Get WAN paths
    resp, _ = client.GetWanPaths(context.Background(), &pb.Empty{})
    for _, path := range resp.WanPaths {
        fmt.Printf("Path %d: %s -> %s\n", path.PathId, path.InterfaceName, path.IpAddress)
        fmt.Printf("  Latency: %d ms, Loss: %.2f%%\n", path.LatencyMs, path.PacketLossPct)
    }
    
    // Get IPsec tunnels
    resp, _ = client.GetIpsecTunnels(context.Background(), &pb.Empty{})
    for _, tun := range resp.Tunnels {
        fmt.Printf("Tunnel %s: %s -> %s (%s)\n", tun.TunnelId, tun.LocalIp, tun.RemoteIp, tun.Status)
        fmt.Printf("  Encryption: %s, Auth: %s\n", tun.Encryption, tun.AuthAlgo)
    }
    
    // Get VRRP instances
    resp, _ = client.GetVrrpInstances(context.Background(), &pb.Empty{})
    for _, inst := range resp.Instances {
        fmt.Printf("VRRP %d: %s -> %s (Priority: %d)\n", inst.InstanceId, inst.VirtualIp, inst.State, inst.Priority)
    }
    
    // Get MPLS LSP
    resp, _ = client.GetMplsLsp(context.Background(), &pb.Empty{})
    for _, lsp := range resp.LspList {
        fmt.Printf("LSP %d: label=%d next_hop=%s iface=%s (%s)\n", lsp.LspId, lsp.Label, lsp.NextHop, lsp.Interface, lsp.Status)
    }
}
```

---

## Уровень реализации для портфолио

### Что показывает этот код:

1. **Работа с Linux kernel interfaces**
   - sysfs (`/sys/class/thermal/`, `/sys/class/net/`)
   - procfs (`/proc/net/xfrm_state`, `/proc/net/xfrm_policy`, `/proc/net/mpls`, `/proc/net/dev`)
   - BFD monitoring — path health per WAN interface
   - keepalived — VRRP instance state monitoring
   - strongSwan — IPsec tunnel monitoring
   - FRRouting — MPLS LSP monitoring
   - iproute2 — MPLS pseudowire management

2. **C++ на уровне embedded**
   - Thread-safe HAL через mutex
   - RAII для управления ресурсами
   - Smart pointers (unique_ptr, shared_ptr)
   - Mock mode для тестирования
   - Signal handling (SIGINT, SIGTERM)

3. **Enterprise телеком-протоколы**
   - SD-WAN — multipath routing, BFD health monitoring, path failover
   - IPsec — XFRM state/policy, strongSwan monitoring, tunnel lifecycle
   - VRRP — keepalived state, master/backup instances, priority management
   - MPLS — LSP state, label management, FRRouting integration

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
- **Понимание Linux internals** — sysfs, procfs, XFRM, BFD, keepalived
- **Thread safety** — mutex, atomic operations
- **Testability** — mock mode для unit-тестов
- **Production-ready** — graceful shutdown, error handling
- **Enterprise telecom domain** — SD-WAN, IPsec, VRRP, MPLS

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

Показывает внешние системы управления, WAN networks, gRPC сервер, HAL слой, Linux kernel subsystems и hardware платформу NXP S32G + Broadcom TomTom.

![](diagrams/C1_Overall_System_Architecture.png)

**Ссылка на PlantUML источник:** [C1_overall_system_architecture.puml](diagrams/C1_overall_system_architecture.puml)

**Описание:**
- **Внешние системы**: NMS/OSS, мониторинг — подключаются к gRPC серверу через protobuf
- **WAN Networks**: Internet 1/2 (PPPoE/DSL), MPLS WAN, LTE Backup
- **gRPC Server**: основной интерфейс API на порту 50055, реализует MtsEnterpriseService
- **HAL Layer**: четыре независимых модуля — SdwanHal, IpsecHal, VrrpHal, MplsHal
- **Linux Kernel**: BFD Monitor, keepalived, strongSwan, FRRouting, network stack
- **Hardware**: NXP S32G SoC, Broadcom TomTom ASIC, 4x 10G SFP+, 8x GE RJ45, WAN interfaces
- **Протоколы**: SNMP, NETCONF/YANG, BGP/OSPF, Telemetry — экспортируют данные из gRPC

### C2: gRPC Service RPC Methods

Описывает все 15 RPC методов (14 unary + 1 streaming), их request/response сообщения и взаимодействие с HAL.

![](diagrams/C2_gRPC_Service_RPC_Methods.png)

**Ссылка на PlantUML источник:** [C2_grpc_service_rpc_methods.puml](diagrams/C2_grpc_service_rpc_methods.puml)

**Описание:**
- **SD-WAN RPC (4)**: GetSdwanStatus, SetSdwanConfig, GetWanPaths, SetPathPriority
- **IPsec RPC (4)**: GetIpsecStatus, GetIpsecTunnels, CreateIpsecTunnel, DeleteIpsecTunnel
- **VRRP RPC (3)**: GetVrrpStatus, GetVrrpInstances, SetVrrpPriority
- **MPLS RPC (4)**: GetMplsStatus, GetMplsLsp, CreateMplsLsp, DeleteMplsLsp
- **Streaming RPC (1)**: SubscribeTelemetry (all HAL)

### C3: HAL Layer Architecture

Детальная архитектура четырёх HAL модулей с их интерфейсами, структурами данных и интеграцией с Linux.

![](diagrams/C3_HAL_Layer_Architecture.png)

**Ссылка на PlantUML источник:** [C3_hal_layer_architecture.puml](diagrams/C3_hal_layer_architecture.puml)

**Описание:**
- **SdwanHal (SD-WAN Path Management)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `getWanPathList()`, `setPathPriority()`, `isAvailable()`
  - Структура: `SdwanStatus` — active_wan_paths, selected_path, bfd_sessions, path_health_score, failover_count
  - Интеграция: sysfs `/sys/class/thermal/`, `/proc/net/`, BFD monitor
- **IpsecHal (IPsec Tunnel Management)**:
  - Интерфейсы: `getStatus()`, `getTunnelList()`, `createTunnel()`, `deleteTunnel()`, `isAvailable()`
  - Структура: `IpsecTunnelInfo` — tunnel_id, local/remote_ip, status, bytes/packets, encryption/auth_algo
  - Интеграция: `/proc/net/xfrm_state`, `/proc/net/xfrm_policy`, strongSwan
- **VrrpHal (VRRP Monitoring)**:
  - Интерфейсы: `getStatus()`, `getInstanceList()`, `setPriority()`, `isAvailable()`
  - Структура: `VrrpInstanceInfo` — instance_id, virtual_ip, priority, state, master_ip, advert_interval
  - Интеграция: keepalived state, sysfs
- **MplsHal (MPLS LSP Management)**:
  - Интерфейсы: `getStatus()`, `getLspList()`, `createLsp()`, `deleteLsp()`, `isAvailable()`
  - Структура: `MplsLspInfo` — lsp_id, label, next_hop, interface, bytes, packets
  - Интеграция: `/proc/net/mpls`, FRRouting

### C4: Linux Integration & Data Flow

Показывает полный путь данных от hardware NXP S32G + Broadcom TomTom через Linux kernel к внешним клиентам.

![](diagrams/C4_Linux_Integration_Data_Flow.png)

**Ссылка на PlantUML источник:** [C4_linux_integration_data_flow.puml](diagrams/C4_linux_integration_data_flow.puml)

**Описание:**
- **Путь данных (Hardware → Client)**:
  1. NXP S32G + Broadcom TomTom hardware → Linux kernel drivers
  2. Kernel (BFD, strongSwan, keepalived, FRRouting, MPLS) → sysfs/procfs
  3. sysfs/procfs → HAL (SdwanHal, IpsecHal, VrrpHal, MplsHal)
  4. HAL → gRPC service (MtsEnterpriseServiceImpl)
  5. gRPC → External clients (NMS/OSS)
- **Путь управления (Client → Hardware)**:
  1. Client → gRPC commands (SetSdwanConfig, CreateIpsecTunnel, SetVrrpPriority)
  2. gRPC → HAL configuration methods
  3. HAL → kernel BFD/strongSwan/keepalived/FRR
  4. Kernel → Hardware configuration
- **Ключевые subsystems**:
  - BFD Monitor — path health per WAN interface
  - strongSwan — IPsec SA management
  - keepalived — VRRP instance state
  - FRRouting — MPLS LSP management

---

## Контакты

Для вопросов по проекту:
- Email: [arifulin@gmail.com]
- GitHub: [https://github/artyomich/mts-router]
