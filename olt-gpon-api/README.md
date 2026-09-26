# MTS-OLT-2000 OLT GPON — gRPC API Backend

## Описание проекта

Реализация **gRPC backend на C++** для маршрутизатора **MTS-OLT-2000** (OLT GPON).
Показывает уровень владения **Linux subsystems**, **C++ API design** и **GPON/OMCI/TR-069 протоколами**.

---

## Что реализовано

### 1. Hardware Abstraction Layer (HAL)

Четыре независимых HAL модуля, демонстрирующих работу с Linux subsystem:

#### GponHal (GPON Line Interface)
- **Чтение из sysfs**: `/sys/class/thermal/`, `/sys/class/power/`, `/sys/class/gpon/` — температура, питание, GPON port state
- **rtl_gpon CLI**: управление GPON портами через `rtl_gpon` command-line tool
- **Парсинг логов**: мониторинг GPON daemon logs
- **Thread-safe**: mutex для защиты port table
- **Мок-режим**: для тестирования без hardware
- **Поддержка**: до 800 ONU на порт

#### OnuHal (ONU Management)
- **Чтение из sysfs**: `/sys/class/gpon/` — ONU config, firmware state
- **SNMP**: мониторинг bandwidth/QoS для ONU
- **Парсинг /proc/net**: статистика ONU connections
- **Thread-safe**: mutex для защиты ONU table
- **Мок-режим**: для тестирования

#### OmciHal (OMCI Protocol Handler)
- **Чтение из /proc/net/omci**: OMCI entity state, MCT/VID mapping
- **Парсинг OMCI daemon logs**: управление OMCI entities
- **Thread-safe**: mutex для защиты entity table
- **Мок-режим**: для тестирования

#### Tr069Hal (TR-069/ACS Monitor)
- **Чтение из /proc/net/tcp**: мониторинг ACS TCP connections
- **Парсинг cwmpd daemon logs**: состояние ACS daemon
- **Thread-safe**: mutex для защиты connection table
- **Мок-режим**: для тестирования

### 2. gRPC Service

Реализация **MtsOltGponService** с 11 RPC методами:

| RPC | Назначение | HAL |
|-----|-----------|-----|
| `GetGponStatus` | Получить статус GPON портов | GponHal |
| `SetGponConfig` | Установить конфигурацию GPON | GponHal |
| `GetGponPorts` | Получить список GPON портов | GponHal |
| `GetOnuList` | Получить список ONU | OnuHal |
| `GetOnuStatus` | Получить статус ONU | OnuHal |
| `SetOnuConfig` | Установить конфигурацию ONU | OnuHal |
| `RestartOnu` | Перезагрузить ONU | OnuHal |
| `GetOmciEntities` | Получить OMCI entities | OmciHal |
| `SetOmciEntity` | Установить OMCI entity | OmciHal |
| `GetTr069Status` | Получить статус TR-069 | Tr069Hal |
| `SubscribeTelemetry` | Streaming telemetry | Все HAL |

### 3. Linux Integration

- **sysfs**: прямое чтение из `/sys/class/thermal/`, `/sys/class/power/`, `/sys/class/gpon/`
- **procfs**: парсинг `/proc/net/omci`, `/proc/net/dev`, `/proc/net/tcp`
- **rtl_gpon CLI**: управление GPON портами и конфигурацией
- **cwmpd monitoring**: мониторинг TR-069 ACS connections
- **SNMP**: bandwidth/QoS monitoring для ONU
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
olt-gpon-api/
├── proto/
│   └── mts_olt_gpon.proto           — gRPC protobuf specification
├── include/
│   ├── hal/
│   │   ├── gpon_hal.h               — GPON HAL interface
│   │   ├── onu_hal.h                — ONU HAL interface
│   │   ├── omci_hal.h               — OMCI HAL interface
│   │   └── tr069_hal.h              — TR-069 HAL interface
│   └── service/
│       └── olt_gpon_service.h       — gRPC service interface
├── src/
│   ├── hal/
│   │   ├── gpon_hal.cpp             — GPON HAL implementation
│   │   ├── onu_hal.cpp              — ONU HAL implementation
│   │   ├── omci_hal.cpp             — OMCI HAL implementation
│   │   └── tr069_hal.cpp            — TR-069 HAL implementation
│   ├── service/
│   │   └── olt_gpon_service.cpp     — gRPC service implementation
│   └── main.cpp                     — Server entry point
├── diagrams/
│   ├── C1_overall_system_architecture.puml
│   ├── C2_grpc_service_rpc_methods.puml
│   ├── C3_hal_layer_architecture.puml
│   └── C4_linux_integration_data_flow.puml
├── tests/
│   ├── test_gpon_hal.cpp            — GPON HAL tests
│   ├── test_onu_hal.cpp             — ONU HAL tests
│   ├── test_omci_hal.cpp            — OMCI HAL tests
│   ├── test_tr069_hal.cpp           — TR-069 HAL tests
│   └── CMakeLists.txt
├── config/
│   └── mts-olt2000.conf             — Server configuration
├── CMakeLists.txt                   — Build configuration
├── Dockerfile                       — Container build
└── README.md                        — Этот файл
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
cd olt-gpon-api
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Запуск
```bash
./mts-olt-gpon-server 0.0.0.0:50053
```

### Тестирование через gRPC CLI
```bash
# Get GPON status
grpcurl -plaintext localhost:50053 mts.oltgpon.v1.MtsOltGponService/GetGponStatus

# Get ONU list
grpcurl -plaintext localhost:50053 mts.oltgpon.v1.MtsOltGponService/GetOnuList

# Get OMCI entities
grpcurl -plaintext localhost:50053 mts.oltgpon.v1.MtsOltGponService/GetOmciEntities

# Get TR-069 status
grpcurl -plaintext localhost:50053 mts.oltgpon.v1.MtsOltGponService/GetTr069Status

# Subscribe to telemetry
grpcurl -plaintext -d '{"paths":["cpu","memory","temperature"]}' \
    localhost:50053 mts.oltgpon.v1.MtsOltGponService/SubscribeTelemetry
```

---

## Примеры использования

### Python клиент
```python
import grpc
import mts_olt_gpon_pb2
import mts_olt_gpon_pb2_grpc

channel = grpc.insecure_channel('localhost:50053')
stub = mts_olt_gpon_pb2_grpc.MtsOltGponServiceStub(channel)

# Get GPON status
response = stub.GetGponStatus(mts_olt_gpon_pb2.Empty())
print(f"GPON Port: {response.gpon_status.port_id}")
print(f"ONU count: {response.gpon_status.onu_count}")
print(f"Max ONU: {response.gpon_status.max_onu}")
print(f"Uplink rate: {response.gpon_status.uplink_rate} Mbps")

# Get ONU list
response = stub.GetOnuList(mts_olt_gpon_pb2.Empty())
for onu in response.onu_list:
    print(f"ONU ID: {onu.onu_id}, MAC: {onu.mac_address}, Status: {onu.status}")

# Subscribe to telemetry
responses = stub.SubscribeTelemetry(
    mts_olt_gpon_pb2.TelemetrySubscription(
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
    pb "path/to/mts_olt_gpon"
)

func main() {
    conn, _ := grpc.Dial("localhost:50053", grpc.WithInsecure())
    defer conn.Close()
    
    client := pb.NewMtsOltGponServiceClient(conn)
    
    // Get GPON status
    resp, _ := client.GetGponStatus(context.Background(), &pb.Empty{})
    fmt.Printf("GPON Port: %d\n", resp.GponStatus.PortId)
    fmt.Printf("ONU count: %d\n", resp.GponStatus.OnuCount)
    fmt.Printf("Max ONU: %d\n", resp.GponStatus.MaxOnu)
    fmt.Printf("Uplink rate: %d Mbps\n", resp.GponStatus.UplinkRate)
    
    // Get ONU list
    resp, _ = client.GetOnuList(context.Background(), &pb.Empty{})
    for _, onu := range resp.OnuList {
        fmt.Printf("ONU ID: %d, MAC: %s, Status: %s\n", onu.OnuId, onu.MacAddress, onu.Status)
    }
}
```

---

## Уровень реализации для портфолио

### Что показывает этот код:

1. **Работа с Linux kernel interfaces**
   - sysfs (`/sys/class/thermal/`, `/sys/class/power/`, `/sys/class/gpon/`)
   - procfs (`/proc/net/omci`, `/proc/net/dev`, `/proc/net/tcp`)
   - rtl_gpon CLI — GPON port management
   - cwmpd monitoring — TR-069 ACS connections
   - SNMP — bandwidth/QoS monitoring

2. **C++ на уровне embedded**
   - Thread-safe HAL через mutex
   - RAII для управления ресурсами
   - Smart pointers (unique_ptr, shared_ptr)
   - Mock mode для тестирования
   - Signal handling (SIGINT, SIGTERM)

3. **GPON/OMCI/TR-069 протоколы**
   - GPON — line interface management, ONU lifecycle
   - OMCI — managed object CRUD, MCT/VID mapping
   - TR-069 — ACS monitoring, connection tracking

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
- **Понимание Linux internals** — sysfs, procfs, CLI integration
- **Thread safety** — mutex, atomic operations
- **Testability** — mock mode для unit-тестов
- **Production-ready** — graceful shutdown, error handling
- **GPON domain** — GPON, OMCI, TR-069

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

Показывает внешние системы управления, gRPC сервер, HAL слой, Linux kernel subsystems и hardware платформу Tofino 2 + RTL960x.

![](diagrams/C1_Overall_System_Architecture.png)

**Ссылка на PlantUML источник:** [C1_overall_system_architecture.puml](diagrams/C1_overall_system_architecture.puml)

**Описание:**
- **Внешние системы**: NMS/OSS, мониторинг — подключаются к gRPC серверу через protobuf
- **gRPC Server**: основной интерфейс API на порту 50053, реализует MtsOltGponService
- **HAL Layer**: четыре независимых модуля — GponHal, OnuHal, OmciHal, Tr069Hal
- **Linux Kernel**: GPON Driver, OMCI Subsystem, cwmpd daemon, network stack
- **Hardware**: Tofino 2 ASIC (forwarding), AMD EPYC CPU, RTL960x GPON PHY, 10G SFP+ ports
- **ONU/ONT**: до 192 GPON ONT и 192 EPON ONT устройств
- **Протоколы**: SNMP, TR-069, OMCI, Telemetry — экспортируют данные из gRPC

### C2: gRPC Service RPC Methods

Описывает все 11 RPC методов (10 unary + 1 streaming), их request/response сообщения и взаимодействие с HAL.

![](diagrams/C2_gRPC_Service_RPC_Methods.png)

**Ссылка на PlantUML источник:** [C2_grpc_service_rpc_methods.puml](diagrams/C2_grpc_service_rpc_methods.puml)

**Описание:**
- **GPON RPC (3)**: GetGponStatus, SetGponConfig, GetGponPorts
- **ONU RPC (4)**: GetOnuList, GetOnuStatus, SetOnuConfig, RestartOnu
- **OMCI RPC (2)**: GetOmciEntities, SetOmciEntity
- **TR-069 RPC (2)**: GetTr069Status, GetACSInfo
- **Streaming RPC (1)**: SubscribeTelemetry (all HAL)

### C3: HAL Layer Architecture

Детальная архитектура четырёх HAL модулей с их интерфейсами, структурами данных и интеграцией с Linux.

![](diagrams/C3_HAL_Layer_Architecture.png)

**Ссылка на PlantUML источник:** [C3_hal_layer_architecture.puml](diagrams/C3_hal_layer_architecture.puml)

**Описание:**
- **GponHal (GPON Line Interface)**:
  - Интерфейсы: `getStatus()`, `setConfig()`, `getPortList()`, `isAvailable()`
  - Структура: `GponStatus` — port_id, onu_count, max_onu, uplink/downlink_rate, tx/rx_power, temperature
  - Интеграция: sysfs `/sys/class/gpon/`, rtl_gpon CLI, GPON daemon logs
- **OnuHal (ONU Management)**:
  - Интерфейсы: `getList()`, `getStatus()`, `setConfig()`, `restart()`, `isAvailable()`
  - Структура: `OnuInfo` — onu_id, mac_address, serial_number, firmware_version, uptime, rx/tx_power, bandwidth
  - Интеграция: sysfs `/sys/class/gpon/`, SNMP bandwidth/QoS
- **OmciHal (OMCI Protocol)**:
  - Интерфейсы: `getEntities()`, `setEntity()`, `isAvailable()`
  - Структура: `OmciEntity` — entity_id, mct, vid, operational/managed/admin_state
  - Интеграция: procfs `/proc/net/omci/`, OMCI daemon
- **Tr069Hal (TR-069/ACS)**:
  - Интерфейсы: `getStatus()`, `getACSInfo()`, `isAvailable()`
  - Структура: `Tr069Status` — acs_count, acs_active, acs_address/port, connection_requests, last_login
  - Интеграция: procfs `/proc/net/tcp/`, cwmpd daemon

### C4: Linux Integration & Data Flow

Показывает полный путь данных от hardware Tofino 2 + RTL960x через Linux kernel к внешним клиентам.

![](diagrams/C4_Linux_Integration_Data_Flow.png)

**Ссылка на PlantUML источник:** [C4_linux_integration_data_flow.puml](diagrams/C4_linux_integration_data_flow.puml)

**Описание:**
- **Путь данных (Hardware → Client)**:
  1. Tofino 2 + RTL960x hardware (GPON PHY, forwarding) → Linux kernel drivers
  2. Kernel (GPON Driver, OMCI Subsystem, cwmpd) → sysfs/procfs
  3. sysfs/procfs → HAL (GponHal, OnuHal, OmciHal, Tr069Hal)
  4. HAL → gRPC service (MtsOltGponServiceImpl)
  5. gRPC → External clients (NMS/OSS)
- **Путь управления (Client → Hardware)**:
  1. Client → gRPC commands (SetGponConfig, SetOnuConfig, SetOmciEntity)
  2. gRPC → HAL configuration methods
  3. HAL → kernel rtl_gpon CLI/ioctl
  4. Kernel → Hardware configuration
- **Ключевые subsystems**:
  - GPON Driver — GPON line interface management
  - OMCI Subsystem — managed object communication
  - cwmpd — TR-069 ACS daemon
  - Network Subsystem — packet processing

---

## Контакты

Для вопросов по проекту:
- Email: [arifulin@gmail.com]
- GitHub: [https://github/artyomich/mts-router]
