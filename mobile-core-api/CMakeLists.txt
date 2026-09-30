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
    ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Define proto file
set(PROTO_SRC proto/mts_mobile_core.proto)

# Generate protobuf C++ code
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

# Find grpc_cpp_plugin
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/bin /usr/local/bin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

# Generate gRPC C++ code
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_mobile_core.grpc.pb.cc
           ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_mobile_core.grpc.pb.h
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}
    COMMENT "Generating gRPC C++ code from ${PROTO_SRC}"
    VERBATIM
)

set(GRPC_SRCS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_mobile_core.grpc.pb.cc)
set(GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_mobile_core.grpc.pb.h)

# Create a custom target for gRPC generation
add_custom_target(grpc_generated ALL
    DEPENDS ${GRPC_SRCS} ${GRPC_HDRS}
)

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
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)

add_executable(mts-mc5000-server ${SOURCES})
add_dependencies(mts-mc5000-server grpc_generated)

target_link_libraries(mts-mc5000-server
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    pthread
    ${Boost_LIBRARIES}
)

# Enable reflection
target_compile_definitions(mts-mc5000-server PRIVATE GRPC_ENABLE_FORK_SUPPORT)

# Set include directories for the target
target_include_directories(mts-mc5000-server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
)

# Install
install(TARGETS mts-mc5000-server DESTINATION bin)
