cmake_minimum_required(VERSION 3.14)
project(mts-cr9000-api VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(gRPC REQUIRED)

# Use FindProtobuf instead of protobufConfig (Ubuntu 24.04 doesn't ship protobuf CMake config)
include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

set(PROTO_SRC proto/mts_core_router.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

# Generate gRPC code using protoc with grpc_cpp_plugin
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/bin /usr/local/bin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_core_router.grpc.pb.cc
         ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_core_router.grpc.pb.h
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
       --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
       --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
       mts_core_router.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}
  COMMENT "Generating gRPC C++ code"
)

set(GRPC_SRCS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_core_router.grpc.pb.cc)
set(GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_core_router.grpc.pb.h)

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
