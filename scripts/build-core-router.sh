#!/bin/bash
# Build script for MTS-CR-9000 Core Router API
# Generates C++ gRPC backend with P4Runtime, BGP, MPLS, SRv6 HAL modules
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DEVICE_DIR="${PROJECT_DIR}/core-router-api"
BUILD_DIR="${DEVICE_DIR}/build"
PROTO_DIR="${DEVICE_DIR}/proto"
INCLUDE_DIR="${DEVICE_DIR}/include"
SRC_DIR="${DEVICE_DIR}/src"
CONFIG_DIR="${DEVICE_DIR}/config"

echo "[CR9000] Building MTS-CR-9000 Core Router API..."

# Create directory structure
mkdir -p "${PROTO_DIR}" "${INCLUDE_DIR}/hal" "${INCLUDE_DIR}/service" \
         "${SRC_DIR}/hal" "${SRC_DIR}/service" "${SRC_DIR}/p4runtime" \
         "${SRC_DIR}/bgp" "${SRC_DIR}/mpls" "${SRC_DIR}/srv6" \
         "${CONFIG_DIR}" "${BUILD_DIR}"

# Generate protobuf definition
cat > "${PROTO_DIR}/mts_core_router.proto" << 'PROTO_EOF'
syntax = "proto3";

package mts.core.router.v1;

option cc_generic_services = true;

// Fabric status
message FabricStatus {
    string name = 1;
    uint32 num_slots = 2;
    repeated SlotStatus slots = 3;
    double total_bandwidth_gbps = 4;
    string status = 5; // active, degraded, offline
}

message SlotStatus {
    uint32 slot_id = 1;
    string card_type = 2; // line-card, control-card, fabric-card
    string status = 3; // up, down, initializing
    double cpu_usage = 4;
    double memory_usage = 5;
}

// Line card status
message LineCardStatus {
    uint32 card_id = 1;
    string asic_type = 2; // tofino2, tomtom
    string status = 3;
    repeated PortStatus ports = 4;
    uint32 active_sessions = 5;
    uint64 packets_forwarded = 6;
    uint64 bytes_forwarded = 7;
    uint64 errors = 8;
}

// Port status
message PortStatus {
    string name = 1;
    string type = 2; // qsfp_dd, qsfp28, sfp_plus
    uint32 speed_mbps = 3;
    string status = 4; // up, down, error
    double rx_power = 5;
    double tx_power = 6;
    double temperature = 7;
    uint64 rx_bytes = 8;
    uint64 tx_bytes = 9;
    uint64 rx_packets = 10;
    uint64 tx_packets = 11;
    uint64 rx_errors = 12;
    uint64 tx_errors = 13;
}

// P4 pipeline status
message P4PipelineStatus {
    string pipeline_id = 1;
    string program_name = 2;
    string version = 3;
    string status = 4; // compiled, loaded, running, error
    uint64 compiled_at = 5;
    uint64 loaded_at = 6;
    uint32 num_tables = 7;
    uint64 num_entries = 8;
}

// SRv6 status
message Srv6Status {
    string sid = 1;
    uint32 sid_length = 2;
    string encap_mode = 3; // end, end.x, end.dx
    string status = 4; // active, inactive
    uint64 packets = 5;
    uint64 bytes = 6;
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

// Device health
message DeviceHealth {
    string device_id = 1;
    string model = 2;
    string firmware = 3;
    double cpu_usage = 4;
    double memory_usage = 5;
    double temperature = 6;
    string status = 7; // healthy, degraded, critical
    uint64 uptime_seconds = 8;
}

// Responses
message FabricStatusResponse {
    FabricStatus fabric = 1;
}

message LineCardStatusResponse {
    repeated LineCardStatus cards = 1;
}

message PortStatusResponse {
    repeated PortStatus ports = 1;
}

message P4PipelineStatusResponse {
    P4PipelineStatus pipeline = 1;
}

message Srv6StatusResponse {
    repeated Srv6Status srv6_entries = 1;
}

message MplsLspStatusResponse {
    repeated MplsLspStatus lsps = 1;
}

message DeviceHealthResponse {
    DeviceHealth health = 1;
}

message CompileP4Request {
    string program = 1;
    string target = 2; // tofino2
}

message CompileP4Response {
    bool success = 1;
    string message = 2;
    string pipeline_id = 3;
}

message SetFabricRequest {
    string config = 1; // JSON config
}

message SetFabricResponse {
    bool success = 1;
    string message = 2;
}

message AddLineCardRequest {
    uint32 slot = 1;
    string card_type = 2;
    string config = 3;
}

message AddLineCardResponse {
    bool success = 1;
    string message = 2;
    uint32 card_id = 3;
}

message CreateLspRequest {
    string name = 1;
    string ingress_label = 2;
    string egress_label = 3;
    string next_hop = 4;
    string interface = 5;
}

message CreateLspResponse {
    bool success = 1;
    string message = 2;
    uint32 lsp_id = 3;
}

// MTS Core Router Service
service MtsCoreRouterService {
    rpc GetFabricStatus(Empty) returns (FabricStatusResponse);
    rpc GetLineCardStatus(Empty) returns (LineCardStatusResponse);
    rpc GetPortStatus(Empty) returns (PortStatusResponse);
    rpc GetDeviceHealth(Empty) returns (DeviceHealthResponse);
    rpc GetP4Pipelines(Empty) returns (P4PipelineStatusResponse);
    rpc CompileP4(CompileP4Request) returns (CompileP4Response);
    rpc GetSrv6Status(Empty) returns (Srv6StatusResponse);
    rpc GetMplsLspStatus(Empty) returns (MplsLspStatusResponse);
    rpc CreateMplsLsp(CreateLspRequest) returns (CreateLspResponse);
    rpc SetFabric(SetFabricRequest) returns (SetFabricResponse);
    rpc AddLineCard(AddLineCardRequest) returns (AddLineCardResponse);
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

echo "[CR9000] Proto file generated."

# Generate CMakeLists.txt
cat > "${DEVICE_DIR}/CMakeLists.txt" << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.14)
project(mts-cr9000-api VERSION 1.0.0 LANGUAGES CXX)

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

set(PROTO_SRC proto/mts_core_router.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

set(SOURCES
    src/hal/fabric_hal.cpp
    src/hal/line_card_hal.cpp
    src/hal/port_hal.cpp
    src/service/core_router_service.cpp
    src/p4runtime/p4_manager.cpp
    src/bgp/bgp_monitor.cpp
    src/mpls/lsp_manager.cpp
    src/srv6/srv6_manager.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

add_executable(mts-cr9000-server ${SOURCES})

target_link_libraries(mts-cr9000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
    ${Boost_LIBRARIES}
)

install(TARGETS mts-cr9000-server DESTINATION bin)
CMAKE_EOF

echo "[CR9000] CMakeLists.txt generated."

# Generate config
cat > "${CONFIG_DIR}/mts-cr9000.conf" << 'CONF_EOF'
{
    "device_id": "MTS-CR-9000-001",
    "model": "MTS-CR-9000",
    "grpc_port": 50051,
    "health_interval_ms": 5000,
    "telemetry_interval_ms": 1000,
    "fabric": {
        "num_slots": 8,
        "type": "crossbar",
        "bandwidth_gbps": 128
    },
    "line_cards": {
        "max_cards": 8,
        "default_type": "tofino2-32x100g"
    },
    "bgp": {
        "local_as": 65001,
        "router_id": "10.0.0.1",
        "hold_time": 90,
        "keepalive": 30
    },
    "mpls": {
        "label_range_start": 16,
        "label_range_end": 1048575,
        "max_lsps": 10000
    },
    "srv6": {
        "sid_base": "2001:db8::",
        "sid_length": 80,
        "encap_mode": "end"
    },
    "logging": {
        "level": "info",
        "file": "/var/log/mts-cr9000.log",
        "max_size_mb": 100
    }
}
CONF_EOF

echo "[CR9000] Config generated."
echo "[CR9000] Core Router API skeleton created at ${DEVICE_DIR}"
echo "[CR9000] To compile: cd ${DEVICE_DIR} && mkdir build && cd build && cmake .. && make"
