#!/bin/bash
# Build script for MTS-MC-5000 Mobile Core API
# Generates C++ gRPC backend with UPF, SMF, PFCP, GTP HAL modules
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEVICE_DIR="${PROJECT_DIR}/mobile-core-api"
BUILD_DIR="${DEVICE_DIR}/build"
PROTO_DIR="${DEVICE_DIR}/proto"
INCLUDE_DIR="${DEVICE_DIR}/include"
SRC_DIR="${DEVICE_DIR}/src"
CONFIG_DIR="${DEVICE_DIR}/config"

echo "[MC5000] Building MTS-MC-5000 Mobile Core API..."

mkdir -p "${PROTO_DIR}" "${INCLUDE_DIR}/hal" "${INCLUDE_DIR}/service" \
         "${SRC_DIR}/hal" "${SRC_DIR}/service" "${SRC_DIR}/upf" \
         "${SRC_DIR}/smf" "${SRC_DIR}/pfcp" "${SRC_DIR}/gtp" \
         "${CONFIG_DIR}" "${BUILD_DIR}"

# Generate protobuf definition
cat > "${PROTO_DIR}/mts_mobile_core.proto" << 'PROTO_EOF'
syntax = "proto3";

package mts.mobile.core.v1;

option cc_generic_services = true;

// UPF status
message UpfStatus {
    string upf_id = 1;
    string status = 2; // active, inactive, error
    uint32 active_sessions = 3;
    uint32 max_sessions = 4;
    uint64 rx_bytes = 5;
    uint64 tx_bytes = 6;
    uint64 rx_packets = 7;
    uint64 tx_packets = 8;
    double cpu_usage = 9;
    double memory_usage = 10;
}

// PDU Session
message PduSession {
    string session_id = 1;
    string ue_ip = 2;
    string upf_ip = 3;
    uint32 teid = 4;
    uint32 qfi = 5;
    uint32 five_qi = 6;
    string pnni = 7;
    string status = 8; // active, inactive, releasing
    int64 created_at = 9;
    int64 last_active = 10;
    uint64 rx_bytes = 11;
    uint64 tx_bytes = 12;
}

// PFCP Session
message PfcpSession {
    string session_id = 1;
    string f_seid = 2;
    string peer_ip = 3;
    string type = 4; // upf, smf
    string status = 5; // established, inactive
    repeated PfcpRule rules = 6;
}

message PfcpRule {
    uint32 rule_id = 1;
    string description = 2;
    string action = 3; // forward, drop, buffer
    uint32 qos_index = 4;
}

// GTP Tunnel
message GtpTunnel {
    string tunnel_id = 1;
    string local_ip = 2;
    string remote_ip = 3;
    uint32 local_teid = 4;
    uint32 remote_teid = 5;
    string type = 6; // gtp-u, gtp-c
    string status = 7; // active, inactive
    uint64 rx_bytes = 8;
    uint64 tx_bytes = 9;
}

// 5QI Configuration
message FiveQIConfig {
    uint32 five_qi = 1;
    string name = 2;
    string resource_type = 3; // GBR, non-GBR, delay-critical
    uint32 priority_level = 4;
    uint32 packet_delay_budget_ms = 5;
    uint32 max_data_burst_size = 6;
    uint32 error_rate = 7;
}

// NRF registry entry
message NrfEntry {
    string nf_id = 1;
    string nf_type = 2; // UPF, SMF, AMF, PCF, UDM, AUSF, NSSF
    string status = 3; // registered, deregistered
    string uri = 4;
    repeated string supported_nfs = 5;
    uint32 priority = 6;
    uint32 capacity = 7;
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
message UpfStatusResponse {
    UpfStatus upf = 1;
}

message PduSessionResponse {
    repeated PduSession sessions = 1;
}

message PfcpSessionResponse {
    repeated PfcpSession sessions = 1;
}

message GtpTunnelResponse {
    repeated GtpTunnel tunnels = 1;
}

message FiveQIConfigResponse {
    repeated FiveQIConfig configs = 1;
}

message NrfRegistryResponse {
    repeated NrfEntry entries = 1;
}

message DeviceHealthResponse {
    DeviceHealth health = 1;
}

message CreatePduSessionRequest {
    string ue_ip = 1;
    string pnni = 2;
    uint32 qfi = 3;
    uint32 five_qi = 4;
    string upf_ip = 5;
    uint32 teid = 6;
}

message CreatePduSessionResponse {
    bool success = 1;
    string message = 2;
    string session_id = 3;
}

message DeletePduSessionRequest {
    string session_id = 1;
}

message DeletePduSessionResponse {
    bool success = 1;
    string message = 2;
}

message CreatePfcpSteeringRequest {
    string session_id = 1;
    string rule_id = 2;
    string selector = 3;
    string action = 4; // forward, drop
    uint32 priority = 5;
}

message CreatePfcpSteeringResponse {
    bool success = 1;
    string message = 2;
}

message UpdateFiveQIRequest {
    uint32 five_qi = 1;
    FiveQIConfig config = 2;
}

message UpdateFiveQIResponse {
    bool success = 1;
    string message = 2;
}

// MTS Mobile Core Service
service MtsMobileCoreService {
    rpc GetUpfStatus(Empty) returns (UpfStatusResponse);
    rpc GetPduSessions(Empty) returns (PduSessionResponse);
    rpc CreatePduSession(CreatePduSessionRequest) returns (CreatePduSessionResponse);
    rpc DeletePduSession(DeletePduSessionRequest) returns (DeletePduSessionResponse);
    rpc GetPfcpSessions(Empty) returns (PfcpSessionResponse);
    rpc CreatePfcpSteering(CreatePfcpSteeringRequest) returns (CreatePfcpSteeringResponse);
    rpc GetGtpTunnels(Empty) returns (GtpTunnelResponse);
    rpc GetFiveQIConfigs(Empty) returns (FiveQIConfigResponse);
    rpc UpdateFiveQI(UpdateFiveQIRequest) returns (UpdateFiveQIResponse);
    rpc GetNrfRegistry(Empty) returns (NrfRegistryResponse);
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

echo "[MC5000] Proto file generated."

# Generate CMakeLists.txt
cat > "${DEVICE_DIR}/CMakeLists.txt" << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.14)
project(mts-mc5000-api VERSION 1.0.0 LANGUAGES CXX)

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

set(PROTO_SRC proto/mts_mobile_core.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/upf_hal.cpp
    src/hal/sm_hal.cpp
    src/hal/pfcp_hal.cpp
    src/hal/gtp_hal.cpp
    src/service/mobile_core_service.cpp
    src/upf/session_manager.cpp
    src/smf/session_controller.cpp
    src/pfcp/pfcp_handler.cpp
    src/gtp/gtp_tunnel.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-mc5000-server ${SOURCES})

target_link_libraries(mts-mc5000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-mc5000-server DESTINATION bin)
CMAKE_EOF

echo "[MC5000] CMakeLists.txt generated."

# Generate config
cat > "${CONFIG_DIR}/mts-mc5000.conf" << 'CONF_EOF'
{
    "device_id": "MTS-MC-5000-001",
    "model": "MTS-MC-5000",
    "grpc_port": 50052,
    "health_interval_ms": 5000,
    "telemetry_interval_ms": 1000,
    "upf": {
        "max_sessions": 100000,
        "default_qfi": 1,
        "max_qfi": 63
    },
    "pfcp": {
        "peer_ip": "192.168.10.1",
        "peer_port": 8805,
        "keepalive_interval_s": 15
    },
    "gtp": {
        "local_ip": "10.64.0.1",
        "local_port": 2152,
        "teid_range_start": 0x10000,
        "teid_range_end": 0xFFFFF
    },
    "nrf": {
        "uri": "nrf.mts5gc.local:80",
        "discovery_interval_s": 300
    },
    "five_qi": {
        "default_5qi": 9,
        "supported_5qi": [1, 2, 65, 66, 67, 69, 71, 72, 73, 74, 75, 76, 77, 82, 83, 84, 85, 86, 90, 91, 92, 93, 94, 95, 96]
    },
    "logging": {
        "level": "info",
        "file": "/var/log/mts-mc5000.log",
        "max_size_mb": 100
    }
}
CONF_EOF

echo "[MC5000] Config generated."
echo "[MC5000] Mobile Core API skeleton created at ${DEVICE_DIR}"
echo "[MC5000] To compile: cd ${DEVICE_DIR} && mkdir build && cd build && cmake .. && make"
