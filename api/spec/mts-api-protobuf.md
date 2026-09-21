# MTS Router — gRPC Protobuf спецификация

## 1. Протокол telemetry

```protobuf
syntax = "proto3";

package mts.telemetry.v1;

import "google/protobuf/timestamp.proto";
import "google/protobuf/any.proto";

// Telemetry data stream
message TelemetryData {
  string device_id = 1;
  string device_model = 2;
  google.protobuf.Timestamp timestamp = 3;
  map<string, double> metrics = 4;
  repeated InterfaceStats interfaces = 5;
  repeated CounterStats counters = 6;
  repeated PortStats port_stats = 7;
}

// Interface statistics
message InterfaceStats {
  string name = 1;
  uint64 rx_bytes = 2;
  uint64 tx_bytes = 3;
  uint64 rx_packets = 4;
  uint64 tx_packets = 5;
  uint64 rx_errors = 6;
  uint64 tx_errors = 7;
  uint64 rx_drops = 8;
  uint64 tx_drops = 9;
  uint32 speed = 10;
  string status = 11;
  double utilization = 12;
}

// Counter statistics
message CounterStats {
  string name = 1;
  uint64 value = 2;
  uint64 reset_time = 3;
}

// Port statistics
message PortStats {
  string port_id = 1;
  string name = 2;
  uint64 rx_bytes = 3;
  uint64 tx_bytes = 4;
  uint64 rx_packets = 5;
  uint64 tx_packets = 6;
  uint64 rx_errors = 7;
  uint64 tx_errors = 8;
  double rx_power = 9;
  double tx_power = 10;
  double temperature = 11;
}

// Telemetry subscription
message TelemetrySubscription {
  repeated string paths = 1;
  int64 sample_interval = 2; // in milliseconds
  Encoding encoding = 3;
  bool delta_encoding = 4;
}

// Telemetry request
message TelemetryRequest {
  string device_id = 1;
  repeated string paths = 2;
  int64 start_time = 3;
  int64 end_time = 4;
}

// Telemetry response
message TelemetryResponse {
  string device_id = 1;
  google.protobuf.Timestamp timestamp = 2;
  map<string, string> data = 3;
}

// Encoding types
enum Encoding {
  ENCODING_UNSPECIFIED = 0;
  ENCODING_PROTO = 1;
  ENCODING_JSON = 2;
  ENCODING_ASCII = 3;
}

// Telemetry service
service MtsTelemetry {
  // Subscribe to telemetry stream
  rpc SubscribeTelemetry(TelemetrySubscription)
      returns (stream TelemetryData);

  // Get telemetry data
  rpc GetTelemetry(TelemetryRequest)
      returns (TelemetryResponse);
}
```

## 2. Протокол конфигурации

```protobuf
syntax = "proto3";

package mts.config.v1;

import "google/protobuf/timestamp.proto";

// Config request
message ConfigRequest {
  string device_id = 1;
  string path = 2; // YANG path
  bytes data = 3; // JSON/XML/YAML encoded
  string format = 4; // JSON, XML, YAML
  bool dry_run = 5;
  string transaction_id = 6;
}

// Config response
message ConfigResponse {
  bytes data = 1;
  string format = 2;
  string transaction_id = 3;
  string status = 4; // success, failed, pending
  string error = 5;
  google.protobuf.Timestamp timestamp = 6;
}

// Config subscription
message ConfigSubscription {
  string device_id = 1;
  string path = 2;
  bool subscribe_all = 3;
}

// Config event
message ConfigEvent {
  string device_id = 1;
  google.protobuf.Timestamp timestamp = 2;
  string path = 3;
  bytes data = 4;
  string operation = 5; // create, update, delete
  string transaction_id = 6;
}

// Config service
service MtsConfig {
  // Get configuration
  rpc GetConfig(ConfigRequest) returns (ConfigResponse);

  // Set configuration
  rpc SetConfig(ConfigRequest) returns (ConfigResponse);

  // Delete configuration
  rpc DeleteConfig(ConfigRequest) returns (ConfigResponse);

  // Subscribe to configuration changes
  rpc SubscribeConfig(ConfigSubscription)
      returns (stream ConfigEvent);
}
```

## 3. Протокол устройств

```protobuf
syntax = "proto3";

package mts.device.v1;

import "google/protobuf/timestamp.proto";
import "google/protobuf/any.proto";

// Device info
message DeviceInfo {
  string device_id = 1;
  string model = 2;
  string serial = 3;
  string firmware = 4;
  string kernel = 5;
  string hostname = 6;
  google.protobuf.Timestamp uptime = 7;
  double cpu_usage = 8;
  double memory_usage = 9;
  double temperature = 10;
  string status = 11; // healthy, degraded, critical
}

// Health check
message HealthCheck {
  string device_id = 1;
  google.protobuf.Timestamp timestamp = 2;
  string status = 3;
  double cpu_usage = 4;
  double memory_usage = 5;
  double temperature = 6;
  repeated ComponentHealth components = 7;
}

// Component health
message ComponentHealth {
  string name = 1;
  string type = 2; // cpu, memory, disk, port, fan, power
  string status = 3; // healthy, degraded, failed
  double value = 4;
  string unit = 5;
}

// Device service
service MtsDevice {
  // Get device info
  rpc GetDeviceInfo(Empty) returns (DeviceInfo);

  // Get health check
  rpc GetHealthCheck(Empty) returns (HealthCheck);

  // Get device status
  rpc GetDeviceStatus(Empty) returns (DeviceStatus);

  // Restart device
  rpc RestartDevice(RestartRequest) returns (RestartResponse);

  // Factory reset
  rpc FactoryReset(Empty) returns (FactoryResetResponse);
}

// Device status
message DeviceStatus {
  string device_id = 1;
  string status = 2;
  double cpu_usage = 3;
  double memory_usage = 4;
  double temperature = 5;
  repeated PortStatus ports = 6;
  repeated ComponentStatus components = 7;
  google.protobuf.Timestamp timestamp = 8;
}

// Port status
message PortStatus {
  string port_id = 1;
  string name = 2;
  string status = 3; // up, down, error
  double speed = 4;
  double rx_power = 5;
  double tx_power = 6;
  double temperature = 7;
}

// Component status
message ComponentStatus {
  string name = 1;
  string type = 2;
  string status = 3;
  double value = 4;
  string unit = 5;
}

// Restart request
message RestartRequest {
  bool confirm = 1;
  string reason = 2;
}

// Restart response
message RestartResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp restart_time = 3;
}

// Factory reset response
message FactoryResetResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp reset_time = 3;
}

// Empty message
message Empty {}
```

## 4. Протокол BGP

```protobuf
syntax = "proto3";

package mts.bgp.v1;

import "google/protobuf/timestamp.proto";

// BGP status
message BgpStatus {
  string device_id = 1;
  uint32 local_as = 2;
  string router_id = 3;
  string state = 4; // idle, connect, active, opensent, openconfirm, established
  google.protobuf.Timestamp uptime = 5;
  uint32 prefixes_received = 6;
  uint32 prefixes_sent = 7;
  repeated NeighborStatus neighbors = 8;
}

// Neighbor status
message NeighborStatus {
  string peer = 1;
  uint32 peer_as = 2;
  string state = 3;
  google.protobuf.Timestamp uptime = 4;
  uint32 prefixes_received = 5;
  uint32 prefixes_sent = 6;
  double rx_bytes = 7;
  double tx_bytes = 8;
}

// BGP route
message BgpRoute {
  string prefix = 1;
  string next_hop = 2;
  string interface = 3;
  string protocol = 4;
  uint32 priority = 5;
  uint32 metric = 6;
  google.protobuf.Timestamp age = 7;
  repeated string path = 8;
  map<string, string> attributes = 9;
}

// BGP service
service MtsBgp {
  // Get BGP status
  rpc GetBgpStatus(Empty) returns (BgpStatus);

  // Get BGP routes
  rpc GetBgpRoutes(BgpRouteRequest) returns (stream BgpRoute);

  // Add BGP neighbor
  rpc AddBgpNeighbor(BgpNeighborRequest) returns (BgpNeighborResponse);

  // Delete BGP neighbor
  rpc DeleteBgpNeighbor(BgpNeighborRequest) returns (BgpNeighborResponse);

  // Update BGP neighbor
  rpc UpdateBgpNeighbor(BgpNeighborRequest) returns (BgpNeighborResponse);
}

// BGP route request
message BgpRouteRequest {
  string device_id = 1;
  string prefix = 2;
  string protocol = 3;
}

// BGP neighbor request
message BgpNeighborRequest {
  string device_id = 1;
  string peer = 2;
  uint32 peer_as = 3;
  string password = 4;
  string description = 5;
  uint32 hold_time = 6;
  uint32 keepalive = 7;
  bool passive = 8;
  bool route_reflector = 9;
  string route_reflector_cluster = 10;
}

// BGP neighbor response
message BgpNeighborResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp timestamp = 3;
}
```

## 5. Протокол GPON

```protobuf
syntax = "proto3";

package mts.gpon.v1;

import "google/protobuf/timestamp.proto";

// ONU info
message OnuInfo {
  string onu_id = 1;
  string serial = 2;
  string mac = 3;
  string pon_port = 4;
  string status = 5; // online, offline, error
  int32 power_level = 6;
  int32 distance = 7;
  uint32 vlan = 8;
  string qos_profile = 9;
  uint32 bandwidth_up = 10;
  uint32 bandwidth_down = 11;
  google.protobuf.Timestamp last_seen = 12;
  google.protobuf.Timestamp created = 13;
}

// ONU status
message OnuStatus {
  string onu_id = 1;
  string status = 2;
  int32 power_level = 3;
  int32 distance = 4;
  uint64 rx_bytes = 5;
  uint64 tx_bytes = 6;
  uint64 rx_packets = 7;
  uint64 tx_packets = 8;
  uint64 rx_errors = 9;
  uint64 tx_errors = 10;
  google.protobuf.Timestamp timestamp = 11;
}

// PON port info
message PonPortInfo {
  string pon_id = 1;
  string name = 2;
  string status = 3; // up, down, error
  uint32 num_onu = 4;
  uint32 max_onu = 5;
  double downstream_rate = 6;
  double upstream_rate = 7;
  double downstream_util = 8;
  double upstream_util = 9;
}

// GPON service
service MtsGpon {
  // Get OLT status
  rpc GetOltStatus(Empty) returns (OltStatus);

  // Get PON ports
  rpc GetPonPorts(Empty) returns (stream PonPortInfo);

  // Get ONU list
  rpc GetOnuList(OnuListRequest) returns (stream OnuInfo);

  // Get ONU status
  rpc GetOnuStatus(OnuStatusRequest) returns (OnuStatus);

  // Update ONU config
  rpc UpdateOnuConfig(OnuConfigRequest) returns (OnuConfigResponse);

  // Reset ONU
  rpc ResetOnu(OnuResetRequest) returns (OnuResetResponse);

  // Get OMCI status
  rpc GetOmcisStatus(Empty) returns (OmcisStatus);

  // Get TR-069 config
  rpc GetTr069Config(Empty) returns (Tr069Config);
}

// OLT status
message OltStatus {
  string device_id = 1;
  string status = 2;
  uint32 total_onu = 3;
  uint32 online_onu = 4;
  uint32 offline_onu = 5;
  uint32 error_onu = 6;
  double temperature = 7;
  double voltage = 8;
  google.protobuf.Timestamp timestamp = 9;
}

// ONU list request
message OnuListRequest {
  string pon_port = 1;
  string status = 2;
  uint32 limit = 3;
  uint32 offset = 4;
}

// ONU status request
message OnuStatusRequest {
  string onu_id = 1;
}

// ONU config request
message OnuConfigRequest {
  string onu_id = 1;
  string pon_port = 2;
  uint32 vlan = 3;
  string qos_profile = 4;
  uint32 bandwidth_up = 5;
  uint32 bandwidth_down = 6;
}

// ONU config response
message OnuConfigResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp timestamp = 3;
}

// ONU reset request
message OnuResetRequest {
  string onu_id = 1;
  bool force = 2;
}

// ONU reset response
message OnuResetResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp timestamp = 3;
}

// OMCI status
message OmcisStatus {
  string device_id = 1;
  string status = 2;
  uint32 active_sessions = 3;
  uint32 total_sessions = 4;
  google.protobuf.Timestamp timestamp = 5;
}

// TR-069 config
message Tr069Config {
  string device_id = 1;
  string url = 2;
  string username = 3;
  string password = 4;
  bool enabled = 5;
  uint32 polling_interval = 6;
  google.protobuf.Timestamp last_poll = 7;
  google.protobuf.Timestamp next_poll = 8;
}
```

## 6. Протокол WiFi

```protobuf
syntax = "proto3";

package mts.wifi.v1;

import "google/protobuf/timestamp.proto";

// WiFi BSS info
message WifiBssInfo {
  string bss_id = 1;
  string ssid = 2;
  string band = 3; // 2.4GHz, 5GHz, 6GHz
  uint32 channel = 4;
  uint32 bandwidth = 5; // 20, 40, 80, 160 MHz
  string security = 6; // none, wep, wpa, wpa2, wpa3
  string mode = 7; // ap, sta, monitor
  string status = 8; // up, down, error
  uint32 num_clients = 9;
  double rx_bytes = 10;
  double tx_bytes = 11;
  double rx_packets = 12;
  double tx_packets = 13;
  double rx_errors = 14;
  double tx_errors = 15;
  google.protobuf.Timestamp timestamp = 16;
}

// WiFi client info
message WifiClientInfo {
  string client_id = 1;
  string mac = 2;
  string ssid = 3;
  string band = 4;
  uint32 channel = 5;
  int32 signal = 6; // dBm
  uint32 rx_rate = 7; // Mbps
  uint32 tx_rate = 8; // Mbps
  double rx_bytes = 9;
  double tx_bytes = 10;
  double rx_packets = 11;
  double tx_packets = 12;
  bool connected = 13;
  google.protobuf.Timestamp last_seen = 14;
  google.protobuf.Timestamp connected_at = 15;
}

// WiFi service
service MtsWifi {
  // Get WiFi status
  rpc GetWifiStatus(Empty) returns (WifiStatus);

  // Get BSS list
  rpc GetBssList(Empty) returns (stream WifiBssInfo);

  // Get BSS info
  rpc GetBssInfo(BssInfoRequest) returns (WifiBssInfo);

  // Update BSS config
  rpc UpdateBssConfig(BssConfigRequest) returns (BssConfigResponse);

  // Get client list
  rpc GetClientList(ClientListRequest) returns (stream WifiClientInfo);

  // Get client info
  rpc GetClientInfo(ClientInfoRequest) returns (WifiClientInfo);

  // Block client
  rpc BlockClient(BlockClientRequest) returns (BlockClientResponse);

  // Unblock client
  rpc UnblockClient(BlockClientRequest) returns (BlockClientResponse);
}

// WiFi status
message WifiStatus {
  string device_id = 1;
  string status = 2;
  uint32 num_bss = 3;
  uint32 num_clients = 4;
  double temperature = 5;
  google.protobuf.Timestamp timestamp = 6;
}

// BSS info request
message BssInfoRequest {
  string bss_id = 1;
}

// BSS config request
message BssConfigRequest {
  string bss_id = 1;
  string ssid = 2;
  uint32 channel = 3;
  uint32 bandwidth = 4;
  string security = 5;
  string password = 6;
  bool guest = 7;
  bool hidden = 8;
}

// BSS config response
message BssConfigResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp timestamp = 3;
}

// Client list request
message ClientListRequest {
  string bss_id = 1;
  string status = 2;
  uint32 limit = 3;
  uint32 offset = 4;
}

// Client info request
message ClientInfoRequest {
  string client_id = 1;
}

// Block client request
message BlockClientRequest {
  string client_id = 1;
}

// Block client response
message BlockClientResponse {
  bool success = 1;
  string message = 2;
  google.protobuf.Timestamp timestamp = 3;
}
```

## 7. Компиляция protobuf

```bash
# Install protoc
apt-get install -y protobuf-compiler

# Install Python gRPC tools
pip install grpcio grpcio-tools

# Compile
protoc \
  --proto_path=. \
  --python_out=. \
  --grpc_python_out=. \
  --pyi_out=. \
  mts/telemetry/v1/telemetry.proto \
  mts/config/v1/config.proto \
  mts/device/v1/device.proto \
  mts/bgp/v1/bgp.proto \
  mts/gpon/v1/gpon.proto \
  mts/wifi/v1/wifi.proto
```
