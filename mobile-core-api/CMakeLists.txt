cmake_minimum_required(VERSION 3.14)
project(mts-mc5000-api VERSION 1.0.0 LANGUAGES CXX)

# Suppress CMP0167 warning: The FindBoost module is removed.
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(gRPC REQUIRED)

include(FindProtobuf)
find_package(Protobuf REQUIRED)

# Set include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Define proto file
set(PROTO_SRC proto/mts_mobile_core.proto)

# Generate protobuf C++ code
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

# Find grpc_cpp_plugin
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/bin /usr/local/bin NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
if(GRPC_CPP_PLUGIN STREQUAL "GRPC_CPP_PLUGIN-NOTFOUND")
    # Try to find in gRPC prefix
    get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)
endif()

if(NOT GRPC_CPP_PLUGIN OR GRPC_CPP_PLUGIN STREQUAL "GRPC_CPP_PLUGIN-NOTFOUND")
    message(FATAL_ERROR "grpc_cpp_plugin not found. Install grpc-dev or gRPC CMake package.")
endif()

message(STATUS "Found grpc_cpp_plugin at: ${GRPC_CPP_PLUGIN}")

# Generate gRPC C++ code
set(GRPC_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated)
file(MAKE_DIRECTORY ${GRPC_GENERATED_DIR})

add_custom_command(
    OUTPUT ${GRPC_GENERATED_DIR}/mts_mobile_core.grpc.pb.cc
           ${GRPC_GENERATED_DIR}/mts_mobile_core.grpc.pb.h
           ${GRPC_GENERATED_DIR}/mts_mobile_core.pb.cc
           ${GRPC_GENERATED_DIR}/mts_mobile_core.pb.h
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --grpc_out=${GRPC_GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
        ${PROTO_SRC}
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SRCS}
        ${GRPC_GENERATED_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HDRS}
        ${GRPC_GENERATED_DIR}/
    DEPENDS ${PROTO_SRC}
    COMMENT "Generating gRPC C++ code from ${PROTO_SRC}"
    VERBATIM
)

set(GRPC_SRCS ${GRPC_GENERATED_DIR}/mts_mobile_core.grpc.pb.cc)
set(GRPC_HDRS ${GRPC_GENERATED_DIR}/mts_mobile_core.grpc.pb.h)
set(PROTO_SRCS_FULL ${GRPC_GENERATED_DIR}/mts_mobile_core.pb.cc)
set(PROTO_HDRS_FULL ${GRPC_GENERATED_DIR}/mts_mobile_core.pb.h)

# Source files
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
    ${GRPC_SRCS}
    ${PROTO_SRCS_FULL}
)

add_executable(mts-mc5000-server ${SOURCES})

target_link_libraries(mts-mc5000-server
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    pthread
    ${Boost_LIBRARIES}
)

# Enable reflection if needed
target_compile_definitions(mts-mc5000-server PRIVATE GRPC_ENABLE_FORK_SUPPORT)

# Set include directories for the target
target_include_directories(mts-mc5000-server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${GRPC_GENERATED_DIR}
)

# Install
install(TARGETS mts-mc5000-server DESTINATION bin)
