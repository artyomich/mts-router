#!/bin/bash
# Build script for MTS-RG-500 Residential Gateway API
# Generates C++ gRPC backend with GPON, WiFi, VoIP, IPTV, TR-069 HAL modules
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEVICE_DIR="${PROJECT_DIR}/residential-gateway-api"
BUILD_DIR="${DEVICE_DIR}/build"
PROTO_DIR="${DEVICE_DIR}/proto"
INCLUDE_DIR="${DEVICE_DIR}/include"
SRC_DIR="${DEVICE_DIR}/src"
CONFIG_DIR="${DEVICE_DIR}/config"

echo "[RG500] Building MTS-RG-500 Residential Gateway API..."

mkdir -p "${PROTO_DIR}" "${INCLUDE_DIR}/hal" "${INCLUDE_DIR}/service" \
         "${SRC_DIR}/hal" "${SRC_DIR}/service" "${SRC_DIR}/gpon" \
         "${SRC_DIR}/wifi" "${SRC_DIR}/voip" "${SRC_DIR}/iptv" \
         "${SRC_DIR}/tr069" \
         "${CONFIG_DIR}" "${BUILD_DIR}"

# Generate protobuf definition
cat > "${PROTO_DIR}/mts_residential.proto" << 'PROTO_EOF'
syntax = "proto3";

package mts.residential.v1;

option cc_generic_services = true;

// GPON ONU status
message GponOnuStatus {
    string onu_id = 1;
    string status = 2; // online, offline, error
    int32 power_level = 3;
    int32 distance = 4;
    string pon_port = 5;
    uint32 vlan = 6;
    uint64 rx_bytes = 7;
    uint64 tx_bytes = 8;
}

// WiFi BSS status
message WifiBssInfo {
    string bss_id = 1;
    string ssid = 2;
    string band = 3; // 2.4ghz, 5ghz
    uint32 channel = 4;
    uint32 bandwidth = 5; // 20, 40, 80 MHz
    string security = 6; // none, wep, wpa, wpa2, wpa3
    string mode = 7; // ap, sta, monitor
    string status = 8; // up, down, error
    uint32 num_clients = 9;
    double rx_bytes = 10;
    double tx_bytes = 11;
    double temperature = 12;
}

// WiFi client
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
    bool connected = 11;
    int64 last_seen = 12;
    int64 connected_at = 13;
}

// VoIP status
message VoipStatus {
    string status = 1; // active, inactive, error
    repeated VoipLine lines = 2;
}

message VoipLine {
    uint32 line_id = 1;
    string status = 2; // idle, ringing, active, busy
    string caller_id = 3;
    string callee_id = 4;
    uint32 duration_seconds = 5;
    string codec = 6; // g711, g729, opus
    uint32 rtp_port = 7;
}

// IPTV status
message IptvStatus {
    string status = 1; // active, inactive, error
    uint32 active_channels = 2;
    uint32 total_channels = 3;
    double bandwidth_mbps = 4;
    repeated IptvChannel channels = 5;
}

message IptvChannel {
    uint32 channel_id = 1;
    string name = 2;
    string multicast_ip = 3;
    uint32 multicast_port = 4;
    string status = 5; // active, inactive
    uint32 viewers = 6;
}

// TR-069 status
message Tr069Status {
    string device_id = 1;
    string url = 2;
    bool enabled = 3;
    uint32 polling_interval = 4;
    int64 last_poll = 5;
    int64 next_poll = 6;
    string status = 7; // active, inactive, error
}

// LAN config
message LanConfig {
    string subnet = 1;
    string gateway = 2;
    string dns_primary = 3;
    string dns_secondary = 4;
    bool dhcp_enabled = 5;
    string dhcp_start = 6;
    string dhcp_end = 7;
    uint32 lease_time_hours = 8;
}

// Parental control
message ParentalControl {
    bool enabled = 1;
    repeated ParentalRule rules = 2;
}

message ParentalRule {
    string rule_id = 1;
    string mac = 2;
    string name = 3;
    repeated string blocked_sites = 4;
    repeated TimeRange active_hours = 5;
}

message TimeRange {
    uint32 start_hour = 1;
    uint32 start_min = 2;
    uint32 end_hour = 3;
    uint32 end_min = 4;
}

// Device health
message DeviceHealth {
    string device_id = 1;
    string model = 2;
    string firmware = 3;
    double cpu_usage = 4;
    double memory_usage = 5;
    double temperature = 6;
    string status = 7;
    uint64 uptime_seconds = 8;
}

// Responses
message GponOnuStatusResponse {
    GponOnuStatus status = 1;
}

message WifiBssInfoResponse {
    repeated WifiBssInfo bss = 1;
}

message WifiClientInfoResponse {
    repeated WifiClientInfo clients = 1;
}

message VoipStatusResponse {
    VoipStatus voip = 1;
}

message IptvStatusResponse {
    IptvStatus iptv = 1;
}

message Tr069StatusResponse {
    Tr069Status tr069 = 1;
}

message LanConfigResponse {
    LanConfig config = 1;
}

message ParentalControlResponse {
    ParentalControl control = 1;
}

message DeviceHealthResponse {
    DeviceHealth health = 1;
}

// Update WiFi
message UpdateWifiRequest {
    string bss_id = 1;
    string ssid = 2;
    uint32 channel = 3;
    uint32 bandwidth = 4;
    string security = 5;
    string password = 6;
    bool guest = 7;
    bool hidden = 8;
}

message UpdateWifiResponse {
    bool success = 1;
    string message = 2;
}

// Update VoIP
message UpdateVoipRequest {
    uint32 line_id = 1;
    string sip_server = 2;
    uint32 sip_port = 3;
    string username = 4;
    string password = 5;
    string codec = 6;
}

message UpdateVoipResponse {
    bool success = 1;
    string message = 2;
}

// Block/Unblock client
message BlockClientRequest {
    string client_id = 1;
}

message BlockClientResponse {
    bool success = 1;
    string message = 2;
}

// MTS Residential Gateway Service
service MtsResidentialService {
    rpc GetGponStatus(Empty) returns (GponOnuStatusResponse);
    rpc GetWifiStatus(Empty) returns (WifiBssInfoResponse);
    rpc UpdateWifi(UpdateWifiRequest) returns (UpdateWifiResponse);
    rpc GetWifiClients(Empty) returns (WifiClientInfoResponse);
    rpc GetVoipStatus(Empty) returns (VoipStatusResponse);
    rpc UpdateVoip(UpdateVoipRequest) returns (UpdateVoipResponse);
    rpc GetIptvStatus(Empty) returns (IptvStatusResponse);
    rpc GetTr069Status(Empty) returns (Tr069StatusResponse);
    rpc GetLanConfig(Empty) returns (LanConfigResponse);
    rpc GetParentalControl(Empty) returns (ParentalControlResponse);
    rpc UpdateParentalControl(ParentalControl) returns (BlockClientResponse);
    rpc BlockClient(BlockClientRequest) returns (BlockClientResponse);
    rpc UnblockClient(BlockClientRequest) returns (BlockClientResponse);
    rpc GetDeviceHealth(Empty) returns (DeviceHealthResponse);
    rpc SubscribeTelemetry(TelemetrySubscription) returns (stream TelemetryData);
}

message Empty {}

message TelemetrySubscription {
    repeated string paths = 1;
    int64 sample_interval = 2;
}

message TelemetryData {
    int64 timestamp = 1;
    map<string, double> metrics = 2;
    repeated PortStats ports = 3;
}

message PortStats {
    string name = 1;
    uint64 rx_bytes = 2;
    uint64 tx_bytes = 3;
    uint64 rx_packets = 4;
    uint64 tx_packets = 5;
    uint64 rx_errors = 6;
    uint64 tx_errors = 7;
}
PROTO_EOF

echo "[RG500] Proto file generated."

# Generate CMakeLists.txt
cat > "${DEVICE_DIR}/CMakeLists.txt" << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.14)
project(mts-rg500-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

set(PROTO_SRC proto/mts_residential.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/gpon_hal.cpp
    src/hal/wifi_hal.cpp
    src/hal/voip_hal.cpp
    src/hal/iptv_hal.cpp
    src/hal/tr069_hal.cpp
    src/service/residential_service.cpp
    src/gpon/gpon_engine.cpp
    src/wifi/wifi_manager.cpp
    src/voip/voip_engine.cpp
    src/iptv/iptv_manager.cpp
    src/tr069/tr069_agent.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-rg500-server ${SOURCES})

target_link_libraries(mts-rg500-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-rg500-server DESTINATION bin)
CMAKE_EOF

echo "[RG500] CMakeLists.txt generated."

# Generate config
cat > "${CONFIG_DIR}/mts-rg500.conf" << 'CONF_EOF'
{
    "device_id": "MTS-RG-500-001",
    "model": "MTS-RG-500",
    "grpc_port": 50055,
    "health_interval_ms": 10000,
    "telemetry_interval_ms": 5000,
    "gpon": {
        "onu_id": "MTS20240001",
        "pon_port": 0
    },
    "wifi": {
        "2.4ghz": {
            "ssid": "MTS_Home_2G",
            "channel": 6,
            "bandwidth": 40,
            "security": "wpa2",
            "enabled": true
        },
        "5ghz": {
            "ssid": "MTS_Home_5G",
            "channel": 36,
            "bandwidth": 80,
            "security": "wpa3",
            "enabled": true
        }
    },
    "voip": {
        "lines": 1,
        "codec": "g711",
        "sip_server": "sip.mts.ru",
        "sip_port": 5060
    },
    "iptv": {
        "enabled": true,
        "multicast_range": "239.0.0.0/8",
        "channels": 50
    },
    "tr069": {
        "url": "acs.mts.ru:7547",
        "polling_interval_s": 600,
        "enabled": true
    },
    "lan": {
        "subnet": "192.168.1.0/24",
        "gateway": "192.168.1.1",
        "dhcp": {
            "enabled": true,
            "start": "192.168.1.100",
            "end": "192.168.1.200",
            "lease_hours": 24
        }
    },
    "parental": {
        "enabled": false
    },
    "logging": {
        "level": "info",
        "file": "/var/log/mts-rg500.log",
        "max_size_mb": 50
    }
}
CONF_EOF

echo "[RG500] Config generated."
echo "[RG500] Residential Gateway API skeleton created at ${DEVICE_DIR}"
echo "[RG500] To compile: cd ${DEVICE_DIR} && mkdir build && cd build && cmake .. && make"
