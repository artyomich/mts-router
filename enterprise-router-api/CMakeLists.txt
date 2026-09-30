cmake_minimum_required(VERSION 3.14)
project(mts-er1000-api VERSION 1.0.0 LANGUAGES CXX)

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

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
    ${PROTOBUF_INCLUDE_DIR}
    ${gRPC_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

set(PROTO_SRC proto/mts_enterprise.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin PATHS /usr/bin /usr/local/bin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_enterprise.grpc.pb.cc
         ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_enterprise.grpc.pb.h
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}/grpc_generated
       --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
       --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
       mts_enterprise.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_SRC}
  COMMENT "Generating gRPC C++ code"
)

set(GRPC_SRCS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_enterprise.grpc.pb.cc)
set(GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR}/grpc_generated/mts_enterprise.grpc.pb.h)

set(SOURCES
    src/hal/sdwan_hal.cpp
    src/hal/mpls_hal.cpp
    src/hal/ipsec_hal.cpp
    src/hal/vrrp_hal.cpp
    src/service/enterprise_service.cpp
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
