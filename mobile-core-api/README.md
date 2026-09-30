# MTS Mobile Core API (MTS-MC-5000)

gRPC API service for the MTS Mobile Core 5G Core Network based on Cavium ThunderX3.

## Overview

The MTS-MC-5000 Mobile Core API provides management and telemetry interfaces for:

- **UPF (User Plane Function)** - User plane session management and packet forwarding
- **SMF (Session Management Function)** - Session and policy control
- **AMF (Access and Mobility Management Function)** - Mobility management
- **PCF (Policy Control Function)** - Policy decision enforcement
- **NRF (Network Repository Function)** - Network function discovery
- **PFCP (Packet Forwarding Control Protocol)** - Session management protocol

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  MTS-MC-5000 API                     │
├─────────────────────────────────────────────────────┤
│  gRPC Service Layer                                  │
│  ┌───────────┬───────────┬───────────┬───────────┐  │
│  │ UPF       │ SMF       │ AMF       │ PCF       │  │
│  │ Service   │ Service   │ Service   │ Service   │  │
│  └─────┬─────┴─────┬─────┴─────┬─────┴─────┬─────┘  │
│        │           │           │           │         │
│  ┌─────┴───────────┴───────────┴───────────┴─────┐  │
│  │               HAL Layer                         │  │
│  │  ThunderX3 NIC   │  DPDK PMD   │  K3s K8s     │  │
│  └───────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────┤
│              ThunderX3 Hardware                       │
│  ┌───────────┬───────────┬───────────────────────┐  │
│  │ NPU Core  │ CXL Hub   │  SmartNIC Engine      │  │
│  └───────────┴───────────┴───────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Building

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Running

```bash
./mts-mc5000-api --port=50052 --config=config/mts-mc5000.conf
```

## API Methods

### UPF

| Method | Description |
|--------|-------------|
| `GetUpfStatus` | Get UPF status |
| `SetUpfConfig` | Configure UPF |
| `GetPduSessions` | Get PDU sessions |
| `CreatePduSession` | Create PDU session |
| `DeletePduSession` | Delete PDU session |

### SMF

| Method | Description |
|--------|-------------|
| `GetSmfStatus` | Get SMF status |
| `SetSmfConfig` | Configure SMF |
| `GetNrfEntries` | Get NRF entries |
| `RegisterNrf` | Register with NRF |

### PFCP

| Method | Description |
|--------|-------------|
| `GetPfcpSessions` | Get PFCP sessions |
| `SetPfcpSession` | Configure PFCP session |
| `DeletePfcpSession` | Delete PFCP session |

### GTP

| Method | Description |
|--------|-------------|
| `GetGtpTunnels` | Get GTP tunnels |
| `SetGtpTunnel` | Configure GTP tunnel |
| `DeleteGtpTunnel` | Delete GTP tunnel |

### 5QI

| Method | Description |
|--------|-------------|
| `Get5QIConfig` | Get 5QI configuration |
| `Set5QIConfig` | Set 5QI configuration |

### Health

| Method | Description |
|--------|-------------|
| `GetDeviceHealth` | Get device health |
| `StreamHealth` | Stream health data |

## Configuration

```ini
[server]
port = 50052
tls_enabled = true
tls_cert = /etc/mts/certs/server.crt
tls_key = /etc/mts/certs/server.key
tls_ca = /etc/mts/certs/ca.crt

[logging]
level = info
file = /var/log/mts-mc5000-api.log

[mtls]
enabled = true
cert_file = /etc/mts/certs/client.crt
key_file = /etc/mts/certs/client.key
ca_file = /etc/mts/certs/ca.crt

[telemetry]
stream_interval_ms = 1000
delta_encoding = true

[hw]
dpdk_socket = 0
dpdk_mbufs = 16384
dpdk_mbuf_size = 4096

[k3s]
enabled = true
kube_config = /etc/rancher/k3s/k3s.yaml
```

## Testing

```bash
# Run unit tests
ctest --output-on-failure

# Run integration tests
./scripts/test-mobile-core.sh
```

## Protobuf Messages

See [`proto/mts_mobile_core.proto`](proto/mts_mobile_core.proto) for full message definitions.

## License

GPL-2.0
