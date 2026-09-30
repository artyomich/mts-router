# MTS Mobile Backhaul API (MTS-MB-3000)

gRPC API service for the MTS Mobile Backhaul router based on NXP S32G3.

## Overview

The MTS-MB-3000 Mobile Backhaul API provides management and telemetry interfaces for:

- **MPLS-TP Tunnel Management** - Create, modify, delete and monitor MPLS-TP tunnels
- **Pseudowire (PW) Management** - Configure and monitor pseudowires
- **PTP (Precision Time Protocol)** - Grandmaster clock configuration and monitoring
- **Port Management** - Interface configuration and statistics
- **DPDK Integration** - High-performance packet processing
- **Device Health** - System health monitoring and telemetry

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  MTS-MB-3000 API                     │
├─────────────────────────────────────────────────────┤
│  gRPC Service Layer                                  │
│  ┌───────────┬───────────┬───────────┬───────────┐  │
│  │ MPLS-TP   │ PTP       │ Port      │ DPDK      │  │
│  │ Service   │ Service   │ Service   │ Service   │  │
│  └─────┬─────┴─────┬─────┴─────┬─────┴─────┬─────┘  │
│        │           │           │           │         │
│  ┌─────┴───────────┴───────────┴───────────┴─────┐  │
│  │               HAL Layer                         │  │
│  │  MPLS-TP Engine  │  PTP Engine  │  DPDK PMD    │  │
│  └───────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────┤
│              S32G3 Hardware                         │
│  ┌───────────┬───────────┬───────────────────────┐  │
│  │ Ethernet  │ PTP HW    │  MPLS-TP Engine       │  │
│  │ MACs      │ Engine    │  (Hardware)            │  │
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
./mts-mb3000-api --port=50051 --config=config/mts-mb3000.conf
```

## API Methods

### MPLS-TP Tunnels

| Method | Description |
|--------|-------------|
| `GetMplsTpTunnels` | Get MPLS-TP tunnel status |
| `SetMplsTpTunnel` | Configure MPLS-TP tunnel |
| `DeleteMplsTpTunnel` | Delete MPLS-TP tunnel |

### MPLS-TP Pseudowires

| Method | Description |
|--------|-------------|
| `GetMplsTpPws` | Get pseudowire status |
| `SetMplsTpPw` | Configure pseudowire |
| `DeleteMplsTpPw` | Delete pseudowire |

### MPLS-TP OAM

| Method | Description |
|--------|-------------|
| `GetMplsTpOam` | Get OAM status |
| `StartMplsTpOam` | Start OAM monitoring |
| `StopMplsTpOam` | Stop OAM monitoring |

### PTP

| Method | Description |
|--------|-------------|
| `GetPtpClocks` | Get PTP clock status |
| `SetPtpProfile` | Configure PTP profile |
| `GetPtpProfile` | Get current PTP profile |

### Ports

| Method | Description |
|--------|-------------|
| `GetPortStatuses` | Get port status |
| `SetPortMode` | Configure port mode |

### DPDK

| Method | Description |
|--------|-------------|
| `GetDpdkPortStats` | Get DPDK port statistics |
| `SetDpdkPortConfig` | Configure DPDK port |

### Health

| Method | Description |
|--------|-------------|
| `GetDeviceHealth` | Get device health |
| `StreamPortStats` | Stream port statistics |
| `StreamHealth` | Stream health data |

## Configuration

```ini
[server]
port = 50051
tls_enabled = false
tls_cert = ""
tls_key = ""
tls_ca = ""

[logging]
level = info
file = /var/log/mts-mb3000-api.log

[mtls]
enabled = true
cert_file = /etc/mts/certs/client.crt
key_file = /etc/mts/certs/client.key
ca_file = /etc/mts/certs/ca.crt

[telemetry]
stream_interval_ms = 1000
delta_encoding = true
```

## Testing

```bash
# Run unit tests
ctest --output-on-failure

# Run integration tests
./scripts/test-mobile-backhaul.sh
```

## Protobuf Messages

See [`proto/mts_mobile_backhaul.proto`](proto/mts_mobile_backhaul.proto) for full message definitions.

## License

GPL-2.0
