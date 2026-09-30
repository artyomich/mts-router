# MTS Router — REST API Gateway

## Описание

Единый REST API Gateway для всех устройств MTS Router. Предоставляет унифицированный
интерфейс управления для всех маршрутизаторов и шлюзов.

## Архитектура

```
┌─────────────────────────────────────────────────────────────┐
│                    REST API GATEWAY                         │
├─────────────────────────────────────────────────────────────┤
│  Auth  │  Rate Limit  │  Routing  │  Logging  │  Metrics   │
├─────────────────────────────────────────────────────────────┤
│  Device Handlers:                                          │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐     │
│  │ CR-9000  │ │ MC-5000  │ │ MB-3000  │ │ OLT-2000 │     │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘     │
│  ┌──────────┐ ┌──────────┐                                  │
│  │ ER-1000  │ │ RG-500   │                                  │
│  └──────────┘ └──────────┘                                  │
└─────────────────────────────────────────────────────────────┘
```

## REST Endpoints

### Device Management
| Method | Path | Description |
|--------|------|-------------|
| GET | /api/v1/devices | List all devices |
| GET | /api/v1/devices/{id} | Get device info |
| PUT | /api/v1/devices/{id} | Update device config |
| DELETE | /api/v1/devices/{id} | Remove device |

### Interface Management
| Method | Path | Description |
|--------|------|-------------|
| GET | /api/v1/interfaces | List interfaces |
| GET | /api/v1/interfaces/{id} | Get interface status |
| PUT | /api/v1/interfaces/{id} | Configure interface |

### Routing
| Method | Path | Description |
|--------|------|-------------|
| GET | /api/v1/routing/bgp | BGP status |
| PUT | /api/v1/routing/bgp | Configure BGP |
| GET | /api/v1/routing/ospf | OSPF status |
| PUT | /api/v1/routing/ospf | Configure OSPF |
| GET | /api/v1/routing/static | Static routes |
| PUT | /api/v1/routing/static | Add/modify static route |

### MPLS
| Method | Path | Description |
|--------|------|-------------|
| GET | /api/v1/mpls/lsp | LSP status |
| PUT | /api/v1/mpls/lsp | Create LSP |
| DELETE | /api/v1/mpls/lsp/{id} | Delete LSP |

### Security
| Method | Path | Description |
|--------|------|-------------|
| GET | /api/v1/security/firewall | Firewall status |
| PUT | /api/v1/security/firewall | Configure firewall |
| GET | /api/v1/security/ipsec | IPsec tunnels |
| PUT | /api/v1/security/ipsec | Configure IPsec |

### Telemetry
| Method | Path | Description |
|--------|------|-------------|
| GET | /api/v1/telemetry/health | Device health |
| GET | /api/v1/telemetry/performance | Performance metrics |
| GET | /api/v1/telemetry/logs | System logs |

### Authentication
| Method | Path | Description |
|--------|------|-------------|
| POST | /api/v1/auth/login | Login |
| POST | /api/v1/auth/logout | Logout |
| POST | /api/v1/auth/refresh | Refresh token |

## API Key Authentication

```bash
curl -H "X-API-Key: your-api-key" https://device/api/v1/devices
```

## mTLS Authentication

```bash
curl --cert client.crt --key client.key --cacert ca.crt \
  https://device/api/v1/devices
```

## Response Format

```json
{
  "status": "success",
  "data": { ... },
  "meta": {
    "timestamp": "2024-01-01T00:00:00Z",
    "request_id": "abc123"
  }
}
```

## Error Format

```json
{
  "status": "error",
  "error": {
    "code": 404,
    "message": "Device not found",
    "details": "No device with ID 'xyz'"
  }
}
```
