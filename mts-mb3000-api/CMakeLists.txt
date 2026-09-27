cmake_minimum_required(VERSION 3.10)
project(mts-mb3000-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(gRPC REQUIRED)

include(FindProtobuf)
find_package(Protobuf REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
)

# Generate protobuf and gRPC sources
set(PROTO_SRC proto/mts_backhaul.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/bin /usr/local/bin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_backhaul.grpc.pb.cc
         ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_backhaul.grpc.pb.h
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
       --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
       --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
       mts_backhaul.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}
  COMMENT "Generating gRPC C++ code"
)

set(GRPC_SRCS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_backhaul.grpc.pb.cc)
set(GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_backhaul.grpc.pb.h)

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
