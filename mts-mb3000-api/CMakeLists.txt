cmake_minimum_required(VERSION 3.10)
project(mts-mb3000-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
)

# Generate protobuf and gRPC sources
set(PROTO_SRC proto/mts_backhaul.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})
grpc_cpp_plugin_location(${CMAKE_CURRENT_BINARY_DIR}/grpc_cpp_plugin)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC})

# Source files
set(SOURCES
    src/hal/ptp_hal.cpp
    src/hal/sync_e_hal.cpp
    src/hal/mpls_hal.cpp
    src/service/backhaul_service.cpp
    src/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)

# Executable
add_executable(mts-mb3000-server ${SOURCES})

# Link libraries
target_link_libraries(mts-mb3000-server
    protobuf::libprotobuf
    gRPC::grpc++
    pthread
)

# Install
install(TARGETS mts-mb3000-server DESTINATION bin)
