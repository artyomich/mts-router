#!/bin/bash
# Build script for MTS-OLT-2000 OLT GPON API
# Generates C++ gRPC backend with GPON, ONU, OMCI, TR-069 HAL modules
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEVICE_DIR="${PROJECT_DIR}/olt-gpon-api"
BUILD_DIR="${DEVICE_DIR}/build"
PROTO_DIR="${DEVICE_DIR}/proto"
INCLUDE_DIR="${DEVICE_DIR}/include"
SRC_DIR="${DEVICE_DIR}/src"
CONFIG_DIR="${DEVICE_DIR}/config"

echo "[OLT2000] Building MTS-OLT-2000 OLT GPON API..."

mkdir -p "${PROTO_DIR}" "${INCLUDE_DIR}/hal" "${INCLUDE_DIR}/service" \
         "${SRC_DIR}/hal" "${SRC_DIR}/service" "${SRC_DIR}/gpon" \
         "${SRC_DIR}/onu" "${SRC_DIR}/omci" "${SRC_DIR}/tr069" \
         "${CONFIG_DIR}" "${BUILD_DIR}"

# Generate protobuf definition
cat > "${PROTO_DIR}/mts_olt_gpon.proto" << 'PROTO_EOF'
syntax = "proto3";

package mts.olt.gpon.v1;

option cc_generic_services = true;

// OLT status
message OltStatus {
    string device_id = 1;
    string status = 2; // active, degraded, offline
    uint32 total_onu = 3;
    uint32 online_onu = 4;
    uint32 offline_onu = 5;
    uint32 error_onu = 6;
    double temperature = 7;
    double voltage = 8;
    uint64 uptime_seconds = 9;
}

// PON port
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
    double optical_power = 10;
}

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
    int64 last_seen = 12;
    int64 created = 13;
    uint64 rx_bytes = 14;
    uint64 tx_bytes = 15;
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
    int64 timestamp = 11;
}

// OMCI status
message OmcisStatus {
    string device_id = 1;
    string status = 2;
    uint32 active_sessions = 3;
    uint32 total_sessions = 4;
    int64 timestamp = 5;
}

// TR-069 config
message Tr069Config {
    string device_id = 1;
    string url = 2;
    string username = 3;
    bool enabled = 4;
    uint32 polling_interval = 5;
    int64 last_poll = 6;
    int64 next_poll = 7;
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
message OltStatusResponse {
    OltStatus olt = 1;
}

message PonPortInfoResponse {
    repeated PonPortInfo pon_ports = 1;
}

message OnuInfoResponse {
    repeated OnuInfo onus = 1;
}

message OnuStatusResponse {
    OnuStatus status = 1;
}

message OmcisStatusResponse {
    OmcisStatus omci = 1;
}

message Tr069ConfigResponse {
    Tr069Config config = 1;
}

message DeviceHealthResponse {
    DeviceHealth health = 1;
}

// Update ONU config
message UpdateOnuConfigRequest {
    string onu_id = 1;
    string pon_port = 2;
    uint32 vlan = 3;
    string qos_profile = 4;
    uint32 bandwidth_up = 5;
    uint32 bandwidth_down = 6;
}

message UpdateOnuConfigResponse {
    bool success = 1;
    string message = 2;
}

// Reset ONU
message ResetOnuRequest {
    string onu_id = 1;
    bool force = 2;
}

message ResetOnuResponse {
    bool success = 1;
    string message = 2;
}

// MTS OLT GPON Service
service MtsOltGponService {
    rpc GetOltStatus(Empty) returns (OltStatusResponse);
    rpc GetPonPorts(Empty) returns (PonPortInfoResponse);
    rpc GetOnuList(OnuListRequest) returns (stream OnuInfo);
    rpc GetOnuStatus(OnuStatusRequest) returns (OnuStatusResponse);
    rpc UpdateOnuConfig(UpdateOnuConfigRequest) returns (UpdateOnuConfigResponse);
    rpc ResetOnu(ResetOnuRequest) returns (ResetOnuResponse);
    rpc GetOmcisStatus(Empty) returns (OmcisStatusResponse);
    rpc GetTr069Config(Empty) returns (Tr069ConfigResponse);
    rpc GetDeviceHealth(Empty) returns (DeviceHealthResponse);
    rpc SubscribeTelemetry(TelemetrySubscription) returns (stream TelemetryData);
}

message Empty {}

message OnuListRequest {
    string pon_port = 1;
    string status = 2;
    uint32 limit = 3;
    uint32 offset = 4;
}

message OnuStatusRequest {
    string onu_id = 1;
}

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

echo "[OLT2000] Proto file generated."

# Generate CMakeLists.txt
cat > "${DEVICE_DIR}/CMakeLists.txt" << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.14)
project(mts-olt2000-api VERSION 1.0.0 LANGUAGES CXX)

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

set(PROTO_SRC proto/mts_olt_gpon.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/gpon_hal.cpp
    src/hal/onu_hal.cpp
    src/hal/omci_hal.cpp
    src/hal/tr069_hal.cpp
    src/service/olt_gpon_service.cpp
    src/gpon/gpon_engine.cpp
    src/onu/onu_manager.cpp
    src/omci/omci_handler.cpp
    src/tr069/tr069_agent.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-olt2000-server ${SOURCES})

target_link_libraries(mts-olt2000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-olt2000-server DESTINATION bin)
CMAKE_EOF

echo "[OLT2000] CMakeLists.txt generated."

# Generate config
cat > "${CONFIG_DIR}/mts-olt2000.conf" << 'CONF_EOF'
{
    "device_id": "MTS-OLT-2000-001",
    "model": "MTS-OLT-2000",
    "grpc_port": 50053,
    "health_interval_ms": 5000,
    "telemetry_interval_ms": 1000,
    "gpon": {
        "num_ports": 16,
        "onu_per_port": 128,
        "downstream_rate_mbps": 2488,
        "upstream_rate_mbps": 1244,
        "split_ratio": 128
    },
    "omci": {
        "max_sessions": 2048,
        "management_entity_base": 1024
    },
    "tr069": {
        "url": "acs.mts.ru:7547",
        "username": "olt001",
        "polling_interval_s": 300,
        "enabled": true
    },
    "wdm": {
        "channel_spacing_nm": 0.8,
        "wavelength_range": "1480-1580"
    },
    "logging": {
        "level": "info",
        "file": "/var/log/mts-olt2000.log",
        "max_size_mb": 100
    }
}
CONF_EOF

echo "[OLT2000] Config generated."
echo "[OLT2000] OLT GPON API skeleton created at ${DEVICE_DIR}"
echo "[OLT2000] To compile: cd ${DEVICE_DIR} && mkdir build && cd build && cmake .. && make"
