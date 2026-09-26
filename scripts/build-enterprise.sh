#!/bin/bash
# Build script for MTS-ER-1000 Enterprise Router API
# Generates C++ gRPC backend with SD-WAN, MPLS, IPsec HAL modules
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEVICE_DIR="${PROJECT_DIR}/enterprise-router-api"
BUILD_DIR="${DEVICE_DIR}/build"
PROTO_DIR="${DEVICE_DIR}/proto"
INCLUDE_DIR="${DEVICE_DIR}/include"
SRC_DIR="${DEVICE_DIR}/src"
CONFIG_DIR="${DEVICE_DIR}/config"

echo "[ER1000] Building MTS-ER-1000 Enterprise Router API..."

mkdir -p "${PROTO_DIR}" "${INCLUDE_DIR}/hal" "${INCLUDE_DIR}/service" \
         "${SRC_DIR}/hal" "${SRC_DIR}/service" "${SRC_DIR}/sdwan" \
         "${SRC_DIR}/mpls" "${SRC_DIR}/ipsec" \
         "${CONFIG_DIR}" "${BUILD_DIR}"

# Generate protobuf definition
cat > "${PROTO_DIR}/mts_enterprise.proto" << 'PROTO_EOF'
syntax = "proto3";

package mts.enterprise.v1;

option cc_generic_services = true;

// SD-WAN status
message SdwanStatus {
    string controller_id = 1;
    string status = 2; // active, inactive, error
    repeated WanPath paths = 3;
    uint32 active_paths = 4;
    uint32 max_paths = 5;
}

message WanPath {
    string path_id = 1;
    string wan_interface = 2;
    string type = 3; // mpls, internet, lte, 5g
    string status = 4; // active, standby, down
    uint32 priority = 5;
    string qos_profile = 6;
    string failover_interface = 7;
    uint64 rx_bytes = 8;
    uint64 tx_bytes = 9;
    double latency_ms = 10;
    double packet_loss_pct = 11;
}

// MPLS LSP status
message MplsLspStatus {
    uint32 lsp_id = 1;
    string name = 2;
    string ingress_label = 3;
    string egress_label = 4;
    string next_hop = 5;
    string interface = 6;
    string status = 7; // up, down, initializing
    uint64 rx_packets = 8;
    uint64 tx_packets = 9;
    uint64 rx_bytes = 10;
    uint64 tx_bytes = 11;
}

// IPsec tunnel status
message IpsecTunnelStatus {
    string tunnel_id = 1;
    string name = 2;
    string peer_ip = 3;
    string local_subnet = 4;
    string remote_subnet = 5;
    string mode = 6; // tunnel, transport
    string status = 7; // up, down, negotiating
    uint32 phase = 8; // 1, 2
    uint64 rx_bytes = 9;
    uint64 tx_bytes = 10;
    uint64 rx_packets = 11;
    uint64 tx_packets = 12;
    int64 established_at = 13;
}

// VRRP status
message VrrpStatus {
    string interface = 1;
    uint32 virtual_router_id = 2;
    string status = 3; // master, backup, init
    double priority = 4;
    string master_ip = 5;
    double preempt_delay = 6;
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
message SdwanStatusResponse {
    SdwanStatus sdwan = 1;
}

message MplsLspStatusResponse {
    repeated MplsLspStatus lsps = 1;
}

message IpsecTunnelResponse {
    repeated IpsecTunnelStatus tunnels = 1;
}

message VrrpStatusResponse {
    repeated VrrpStatus vrrps = 1;
}

message DeviceHealthResponse {
    DeviceHealth health = 1;
}

// Create SD-WAN path
message CreateSdwanPathRequest {
    string wan_interface = 1;
    string type = 2; // mpls, internet, lte, 5g
    uint32 priority = 3;
    string qos_profile = 4;
    string failover_interface = 5;
}

message CreateSdwanPathResponse {
    bool success = 1;
    string message = 2;
    string path_id = 3;
}

// Update SD-WAN path
message UpdateSdwanPathRequest {
    string path_id = 1;
    string wan_interface = 2;
    string type = 3;
    uint32 priority = 4;
    string qos_profile = 5;
    string failover_interface = 6;
}

message UpdateSdwanPathResponse {
    bool success = 1;
    string message = 2;
}

// Create MPLS LSP
message CreateMplsLspRequest {
    string name = 1;
    string ingress_label = 2;
    string egress_label = 3;
    string next_hop = 4;
    string interface = 5;
}

message CreateMplsLspResponse {
    bool success = 1;
    string message = 2;
    uint32 lsp_id = 3;
}

// Create IPsec tunnel
message CreateIpsecTunnelRequest {
    string name = 1;
    string peer_ip = 2;
    string local_subnet = 3;
    string remote_subnet = 4;
    string mode = 5; // tunnel, transport
    string encryption = 6; // aes-256, aes-128
    string auth = 7; // sha256, sha1
    string key = 8;
}

message CreateIpsecTunnelResponse {
    bool success = 1;
    string message = 2;
    string tunnel_id = 3;
}

// MTS Enterprise Router Service
service MtsEnterpriseService {
    rpc GetSdwanStatus(Empty) returns (SdwanStatusResponse);
    rpc CreateSdwanPath(CreateSdwanPathRequest) returns (CreateSdwanPathResponse);
    rpc UpdateSdwanPath(UpdateSdwanPathRequest) returns (UpdateSdwanPathResponse);
    rpc GetMplsLspStatus(Empty) returns (MplsLspStatusResponse);
    rpc CreateMplsLsp(CreateMplsLspRequest) returns (CreateMplsLspResponse);
    rpc GetIpsecTunnels(Empty) returns (IpsecTunnelResponse);
    rpc CreateIpsecTunnel(CreateIpsecTunnelRequest) returns (CreateIpsecTunnelResponse);
    rpc GetVrrpStatus(Empty) returns (VrrpStatusResponse);
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

echo "[ER1000] Proto file generated."

# Generate CMakeLists.txt
cat > "${DEVICE_DIR}/CMakeLists.txt" << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.14)
project(mts-er1000-api VERSION 1.0.0 LANGUAGES CXX)

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

set(PROTO_SRC proto/mts_enterprise.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/sdwan_hal.cpp
    src/hal/mpls_hal.cpp
    src/hal/ipsec_hal.cpp
    src/hal/vrrp_hal.cpp
    src/service/enterprise_service.cpp
    src/sdwan/path_manager.cpp
    src/mpls/lsp_manager.cpp
    src/ipsec/tunnel_manager.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-er1000-server ${SOURCES})

target_link_libraries(mts-er1000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-er1000-server DESTINATION bin)
CMAKE_EOF

echo "[ER1000] CMakeLists.txt generated."

# Generate config
cat > "${CONFIG_DIR}/mts-er1000.conf" << 'CONF_EOF'
{
    "device_id": "MTS-ER-1000-001",
    "model": "MTS-ER-1000",
    "grpc_port": 50054,
    "health_interval_ms": 5000,
    "telemetry_interval_ms": 1000,
    "sdwan": {
        "controller_id": "mts-sdwan-ctrl-001",
        "max_paths": 256,
        "path_selection": "application-aware",
        "failover_delay_ms": 50
    },
    "mpls": {
        "label_range_start": 16,
        "label_range_end": 1048575,
        "max_lsps": 5000
    },
    "ipsec": {
        "max_tunnels": 128,
        "default_encryption": "aes-256",
        "default_auth": "sha256"
    },
    "vrrp": {
        "default_priority": 100,
        "default_preempt_delay": 0
    },
    "logging": {
        "level": "info",
        "file": "/var/log/mts-er1000.log",
        "max_size_mb": 100
    }
}
CONF_EOF

echo "[ER1000] Config generated."
echo "[ER1000] Enterprise Router API skeleton created at ${DEVICE_DIR}"
echo "[ER1000] To compile: cd ${DEVICE_DIR} && mkdir build && cd build && cmake .. && make"
