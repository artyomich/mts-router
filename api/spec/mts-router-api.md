# МТС Router — Единый API спецификация

## Обзор

Единый REST/gRPC API для управления всеми устройствами линейки МТС маршрутизаторов:
- MTS-CR-9000 (Core Router)
- MTS-MC-5000 (Mobile Core)
- MTS-MB-3000 (Mobile Backhaul)
- MTS-OLT-2000 (OLT GPON)
- MTS-ER-1000 (Enterprise Router)
- MTS-RG-500 (Residential Gateway)

---

## 1. Общие принципы

### 1.1 Протоколы

| Протокол | Назначение |
|----------|------------|
| REST/JSON | Управление, конфигурация |
| gRPC/protobuf | Telemetry, streaming |
| NETCONF/YANG | Конфигурация (legacy) |
| WebSockets | Real-time events |

### 1.2 Authentication

| Метод | Поддержка |
|-------|-----------|
| mTLS (mutual TLS) | ✓ Все устройства |
| API Key | ✓ Все устройства |
| OAuth2 | ✓ Core devices |
| JWT | ✓ All |

### 1.3 Versioning

```
/api/v1/...
/api/v2/...
```

### 1.4 Error format

```json
{
  "error": {
    "code": 400,
    "message": "Invalid configuration",
    "details": [
      {
        "field": "vlan",
        "issue": "out of range"
      }
    ]
  }
}
```

---

## 2. Общие эндпоинты (все устройства)

### 2.1 Health

```
GET /api/v1/health
```

Response:
```json
{
  "status": "healthy",
  "uptime": 86400,
  "cpu": 45.2,
  "memory": 62.1,
  "temperature": 42
}
```

### 2.2 System info

```
GET /api/v1/system
```

Response:
```json
{
  "model": "MTS-CR-9000",
  "serial": "MTS20240001",
  "firmware": "1.0.0",
  "kernel": "6.6.0",
  "hostname": "mts-cr001",
  "time": "2024-01-15T10:00:00Z"
}
```

### 2.3 Interfaces

```
GET    /api/v1/interfaces
PUT    /api/v1/interfaces/<id>
GET    /api/v1/interfaces/<id>/stats
```

Response:
```json
{
  "interfaces": [
    {
      "name": "eth0",
      "type": "ethernet",
      "status": "up",
      "mac": "aa:bb:cc:dd:ee:ff",
      "ipv4": "192.168.1.1/24",
      "speed": "10G",
      "duplex": "full",
      "stats": {
        "rx_bytes": 1234567890,
        "tx_bytes": 987654321,
        "rx_packets": 1234567,
        "tx_packets": 987654,
        "rx_errors": 0,
        "tx_errors": 0
      }
    }
  ]
}
```

### 2.4 Routing

```
GET    /api/v1/routing/tables
GET    /api/v1/routing/tables/<id>
POST   /api/v1/routing/tables/<id>/routes
DELETE /api/v1/routing/tables/<id>/routes/<id>
```

Response:
```json
{
  "routes": [
    {
      "prefix": "10.0.0.0/8",
      "next_hop": "192.168.1.1",
      "interface": "eth0",
      "protocol": "static",
      "priority": 10,
      "metric": 0,
      "age": 3600
    }
  ]
}
```

### 2.5 BGP

```
GET    /api/v1/bgp
GET    /api/v1/bgp/neighbors
POST   /api/v1/bgp/neighbors
DELETE /api/v1/bgp/neighbors/<id>
PUT    /api/v1/bgp/neighbors/<id>
```

Response:
```json
{
  "bgp": {
    "as": 65001,
    "router_id": "10.0.0.1",
    "state": "established",
    "neighbors": [
      {
        "peer": "192.168.1.2",
        "as": 65002,
        "state": "established",
        "prefixes_received": 100,
        "prefixes_sent": 50,
        "uptime": 86400
      }
    ]
  }
}
```

### 2.6 Telemetry (gRPC streaming)

```
GET /api/v1/telemetry/stream
```

Response (protobuf):
```protobuf
message TelemetryData {
  string device_id = 1;
  int64 timestamp = 2;
  map<string, double> metrics = 3;
  repeated InterfaceStats interfaces = 4;
}

message InterfaceStats {
  string name = 1;
  uint64 rx_bytes = 2;
  uint64 tx_bytes = 3;
  uint64 rx_packets = 4;
  uint64 tx_packets = 5;
  uint64 rx_errors = 6;
  uint64 tx_errors = 7;
}
```

### 2.7 Configuration backup/restore

```
GET    /api/v1/config/backup
POST   /api/v1/config/backup
POST   /api/v1/config/restore
```

### 2.8 Firmware update

```
GET    /api/v1/firmware/status
POST   /api/v1/firmware/upload
POST   /api/v1/firmware/apply
```

### 2.9 Logging

```
GET    /api/v1/logs
GET    /api/v1/logs/<id>
```

### 2.10 Users

```
GET    /api/v1/users
POST   /api/v1/users
PUT    /api/v1/users/<id>
DELETE /api/v1/users/<id>
```

---

## 3. Device-specific API

### 3.1 Core Router (MTS-CR-9000)

```
GET    /api/v1/cr/fabric
PUT    /api/v1/cr/fabric
GET    /api/v1/cr/line-cards
POST   /api/v1/cr/line-cards
GET    /api/v1/cr/p4/pipelines
PUT    /api/v1/cr/p4/pipelines/<id>
POST   /api/v1/cr/p4/compile
GET    /api/v1/cr/sr6
PUT    /api/v1/cr/sr6
GET    /api/v1/cr/mpls/lsps
POST   /api/v1/cr/mpls/lsps
```

### 3.2 Mobile Core (MTS-MC-5000)

```
GET    /api/v1/mc/upf
POST   /api/v1/mc/upf/sessions
DELETE /api/v1/mc/upf/sessions/<id>
GET    /api/v1/mc/pfcp
POST   /api/v1/mc/pfcp/steering
GET    /api/v1/mc/gtp
GET    /api/v1/mc/5qi
PUT    /api/v1/mc/5qi/<id>
GET    /api/v1/mc/nrf
GET    /api/v1/mc/nf/registry
```

### 3.3 Mobile Backhaul (MTS-MB-3000)

```
GET    /api/v1/mb/sync
PUT    /api/v1/mb/sync/grandmaster
GET    /api/v1/mb/ptp
GET    /api/v1/mb/mpls-tp
POST   /api/v1/mb/mpls-tp/pe
GET    /api/v1/mb/sync-e
```

### 3.4 OLT GPON (MTS-OLT-2000)

```
GET    /api/v1/olt
GET    /api/v1/olt/pon-ports
GET    /api/v1/olt/onu
GET    /api/v1/olt/onu/<id>
PUT    /api/v1/olt/onu/<id>
POST   /api/v1/olt/onu/<id>/reset
GET    /api/v1/olt/omci
GET    /api/v1/olt/tr069
GET    /api/v1/olt/wdm
```

### 3.5 Enterprise Router (MTS-ER-1000)

```
GET    /api/v1/er/sdwan
POST   /api/v1/er/sdwan/path
PUT    /api/v1/er/sdwan/path/<id>
GET    /api/v1/er/mpls
POST   /api/v1/er/mpls/lsp
GET    /api/v1/er/ipsec
POST   /api/v1/er/ipsec/tunnel
GET    /api/v1/er/vrrp
```

### 3.6 Residential Gateway (MTS-RG-500)

```
GET    /api/v1/rg/gpon
GET    /api/v1/rg/wifi
PUT    /api/v1/rg/wifi/<band>
GET    /api/v1/rg/voip
PUT    /api/v1/rg/voip
GET    /api/v1/rg/lan
PUT    /api/v1/rg/lan
GET    /api/v1/rg/iptv
GET    /api/v1/rg/tr069
GET    /api/v1/rg/parental
PUT    /api/v1/rg/parental
```

---

## 4. YANG-модели (общие)

### 4.1 Структура

```
mts-router/
├── mts-common.yang           — Общие модели
├── mts-interface.yang        — Interfaces
├── mts-routing.yang          — Routing
├── mts-bgp.yang              — BGP
├── mts-qos.yang              — QoS
├── mts-telemetry.yang        — Telemetry
├── mts-ha.yang               — HA
├── mts-firmware.yang         — Firmware
├── mts-users.yang            — Users
├── mts-cr.yang               — Core Router
├── mts-mc.yang               — Mobile Core
├── mts-mb.yang               — Mobile Backhaul
├── mts-olt.yang              — OLT GPON
├── mts-er.yang               — Enterprise Router
└── mts-rg.yang               — Residential Gateway
```

### 4.2 mts-common.yang (фрагмент)

```yang
module mts-common {
  namespace "urn:mts:router:common";
  prefix "mts";

  import ietf-inet-types { prefix "inet"; }
  import ietf-yang-types { prefix "yang"; }

  container mts {
    container system {
      leaf hostname {
        type string;
      }
      leaf domain-name {
        type string;
      }
      container time {
        leaf utc-offset {
          type string;
        }
        leaf timezone {
          type string;
        }
      }
    }
    container interfaces {
      list interface {
        key name;
        leaf name {
          type string;
        }
        leaf description {
          type string;
        }
        leaf enabled {
          type boolean;
        }
      }
    }
  }
}
```

---

## 5. gRPC сервисы

### 5.1 Telemetry service

```protobuf
service MtsTelemetry {
  rpc SubscribeTelemetry(TelemetrySubscription)
      returns (stream TelemetryData);
  rpc GetTelemetry(TelemetryRequest)
      returns (TelemetryResponse);
}

message TelemetrySubscription {
  repeated string paths = 1;
  int64 sample_interval = 2;
  bool encoding = 3; // 0=PROTO, 1=JSON, 2=ASCII
}

message TelemetryRequest {
  string device_id = 1;
  repeated string paths = 2;
}

message TelemetryResponse {
  string device_id = 1;
  int64 timestamp = 2;
  map<string, string> data = 3;
}
```

### 5.2 Config service

```protobuf
service MtsConfig {
  rpc GetConfig(ConfigRequest) returns (ConfigResponse);
  rpc SetConfig(ConfigRequest) returns (ConfigResponse);
  rpc DeleteConfig(ConfigRequest) returns (ConfigResponse);
  rpc SubscribeConfig(ConfigSubscription)
      returns (stream ConfigEvent);
}

message ConfigRequest {
  string device_id = 1;
  string path = 2;
  bytes data = 3;
  string format = 4; // JSON, XML, YAML
}

message ConfigResponse {
  bytes data = 1;
  string format = 2;
}

message ConfigSubscription {
  string device_id = 1;
  string path = 2;
}

message ConfigEvent {
  string device_id = 1;
  int64 timestamp = 2;
  string path = 3;
  bytes data = 4;
  string operation = 5; // create, update, delete
}
```

---

## 6. Примеры использования

### 6.1 Добавление маршрута

```bash
curl -X POST https://mts-cr001/api/v1/routing/tables/default/routes \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "prefix": "10.0.0.0/8",
    "next_hop": "192.168.1.1",
    "interface": "eth0",
    "protocol": "static",
    "priority": 10
  }'
```

### 6.2 Получение telemetry

```bash
curl https://mts-cr001/api/v1/telemetry/stream \
  -H "Authorization: Bearer <token>" \
  -H "Accept: application/x-protobuf"
```

### 6.3 Создание PDU session (Mobile Core)

```bash
curl -X POST https://mts-mc001/api/v1/mc/upf/sessions \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "ue_ip": "10.64.0.1",
    "pnni": "ims.mts.ru",
    "qfi": 1,
    "5qi": 9,
    "upf_ip": "192.168.10.1",
    "teid": 0x12345
  }'
```

### 6.4 Конфигурация ONU (OLT)

```bash
curl -X PUT https://mts-olt001/api/v1/olt/onu/001 \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "pon_port": 1,
    "onu_id": 10,
    "vlan": 100,
    "qos_profile": "gold",
    "bandwidth_up": "100 Mbps",
    "bandwidth_down": "300 Mbps"
  }'
```

---

## 7. Checklist для разработки API

- [ ] Определить общую схему YANG
- [ ] Реализовать REST API gateway
- [ ] Реализовать gRPC telemetry
- [ ] Реализовать gRPC config
- [ ] Добавить mTLS аутентификацию
- [ ] Добавить API key аутентификацию
- [ ] Добавить rate limiting
- [ ] Добавить API versioning
- [ ] Добавить OpenAPI/Swagger документацию
- [ ] Добавить gRPC protobuf спецификации
- [ ] Написать client SDK (Python, Go, Java)
- [ ] Добавить examples
- [ ] Добавить тесты
