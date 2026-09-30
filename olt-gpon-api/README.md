# MTS OLT GPON API (MTS-OLT-2000)

gRPC API service for the MTS OLT GPON based on Broadcom Tofino 2.

## Overview

The MTS-OLT-2000 OLT GPON API provides management and telemetry interfaces for:

- **GPON OLT Management** - Port and ONU management
- **OMCI (ONT Management and Control Interface)** - ONU configuration and monitoring
- **TR-069 (CWMP)** - Remote management protocol
- **WDM Monitoring** - Wavelength division multiplexing monitoring
- **P4 Pipeline Management** - Programmable data plane
- **YANG/NETCONF** - Configuration management

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  MTS-OLT-2000 API                     │
├─────────────────────────────────────────────────────┤
│  gRPC Service Layer                                  │
│  ┌───────────┬───────────┬───────────┬───────────┐  │
│  │ GPON      │ OMCI      │ TR-069    │ WDM       │  │
│  │ Service   │ Service   │ Service   │ Service   │  │
│  └─────┬─────┴─────┬─────┴─────┬─────┴─────┬─────┘  │
│        │           │           │           │         │
│  ┌─────┴───────────┴───────────┴───────────┴─────┐  │
│  │               HAL Layer                         │  │
│  │  Tofino 2 ASIC   │  P4 Pipeline  │  RTL960x   │  │
│  └───────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────┤
│              Tofino 2 Hardware                        │
│  ┌───────────┬───────────┬───────────────────────┐  │
│  │ P4 Parser │ Forward.  │  Control Plane         │  │
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
./mts-olt2000-api --port=50053 --config=config/mts-olt2000.conf
```

## API Methods

### GPON

| Method | Description |
|--------|-------------|
| `GetGponPorts` | Get GPON port status |
| `SetGponPort` | Configure GPON port |
| `DeleteGponPort` | Delete GPON port |
| `StreamGponPortStats` | Stream port statistics |

### ONU

| Method | Description |
|--------|-------------|
| `GetOnuList` | Get ONU list |
| `GetOnuStatus` | Get ONU status |
| `EnableOnu` | Enable ONU |
| `DisableOnu` | Disable ONU |
| `DeleteOnu` | Delete ONU |

### OMCI

| Method | Description |
|--------|-------------|
| `GetOmciConfig` | Get OMCI configuration |
| `SetOmciConfig` | Set OMCI configuration |
| `SendOmciMsg` | Send OMCI message |
| `GetOmciEvents` | Get OMCI events |

### TR-069

| Method | Description |
|--------|-------------|
| `GetTr069Status` | Get TR-069 status |
| `SetTr069Config` | Configure TR-069 |
| `GetFirmwareStatus` | Get firmware download status |
| `TriggerFirmwareDownload` | Trigger firmware download |

### WDM

| Method | Description |
|--------|-------------|
| `GetWdmStatus` | Get WDM status |
| `SetWdmChannel` | Configure WDM channel |
| `GetWdmMonitor` | Get WDM monitoring data |

### P4 Pipeline

| Method | Description |
|--------|-------------|
| `GetPipelineStatus` | Get P4 pipeline status |
| `LoadPipeline` | Load P4 pipeline |
| `GetPipelineStats` | Get pipeline statistics |

### Health

| Method | Description |
|--------|-------------|
| `GetDeviceHealth` | Get device health |
| `StreamHealth` | Stream health data |

## Configuration

```ini
[server]
port = 50053
tls_enabled = true
tls_cert = /etc/mts/certs/server.crt
tls_key = /etc/mts/certs/server.key
tls_ca = /etc/mts/certs/ca.crt

[logging]
level = info
file = /var/log/mts-olt2000-api.log

[mtls]
enabled = true
cert_file = /etc/mts/certs/client.crt
key_file = /etc/mts/certs/client.key
ca_file = /etc/mts/certs/ca.crt

[telemetry]
stream_interval_ms = 1000
delta_encoding = true

[hw]
tofino_socket = 0
max_gpon_ports = 8
max_onu_per_port = 128

[gpon]
max_ports = 8
max_onu = 128
tx_power_min = -40
tx_power_max = 60
mtu = 1500

[omci]
enabled = true
max_sessions = 1024
event_buffer_size = 4096

[tr069]
enabled = true
port = 7547
https_port = 443
interval_sec = 60
max_connections = 10
cwmp_username = admin
cwmp_password = 
acs_url = https://acs.provider.com
acs_port = 443
acs_username = 
acs_password = 
periodic_interval = 3600
periodic_interval_enabled = false

[wdm]
max_channels = 4
monitor_interval_ms = 1000

[p4]
pipeline_dir = /etc/mts/p4
default_pipeline = mts-gpon.p4info.pb
load_on_start = true
```

## Testing

```bash
# Run unit tests
ctest --output-on-failure

# Run integration tests
./scripts/test-olt-gpon.sh
```

## Protobuf Messages

See [`proto/mts_olt_gpon.proto`](proto/mts_olt_gpon.proto) for full message definitions.

## License

GPL-2.0
